#include <filesystem>
#include <fstream>
#include <glaze/glaze.hpp>
#include <string>
#include <string_view>
#include <type_traits>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

template <typename T>
std::string to_ctypes(std::vector<std::string> &deps) {
    if constexpr (std::is_same_v<T, int>) return "ctypes.c_int";
    else if constexpr (std::is_same_v<T, double>) return "ctypes.c_double";
    else if constexpr (std::is_same_v<T, float>) return "ctypes.c_float";
    else if constexpr (std::is_same_v<T, bool>) return "ctypes.c_bool";
    else if constexpr (std::is_same_v<T, char>) return "ctypes.c_char";
    // Handle Arrays: int[5] -> ctypes.c_int * 5
    else if constexpr (std::is_array_v<T>) {
        return std::string(to_ctypes<std::remove_all_extents_t<T>>(deps)) 
               + " * " + std::to_string(std::extent_v<T>);
    }
    // Handle Nested Glaze Structs
    else if constexpr (requires { glz::meta<T>::value; }) {
        std::string tmp(glz::name_v<T>);
        deps.push_back(tmp);
        return tmp; // Assume the class is defined elsewhere in Python
    }
    #pragma warning("Unsupported type for ctypes mapping: " glz::name_v<T>)
    else return "ctypes.c_void_p";
}


template <typename T> struct extract_member_type;

template <typename Class, typename Type> struct extract_member_type<Type Class::*> {
    using type = Type;
};

template <typename T> void generate_python_binding(std::ostream &out, std::vector<std::string> &deps) {
    static constexpr auto info = glz::reflect<T>{};

    std::string type_name = std::string(glz::meta<T>::name);

    out << "import ctypes\n\n";
    out << "class " << type_name << "(ctypes.Structure):\n";
    out << "    _fields_ = [\n";

    glz::for_each<info.size>([&out, &deps]<auto I>() {
        using FieldTypePtr = std::decay_t<decltype(glz::get<I>(info.values))>;
        using FieldType = typename extract_member_type<FieldTypePtr>::type;
        std::string_view field_name = info.keys[I];
        std::string ctypes_type = to_ctypes<FieldType>(deps);
        out << "        ('" << field_name << "', " << ctypes_type << "),\n";
    });

    out << "    ]\n";
}

template <typename T>
concept CtypesMappable = requires { 
    glz::meta<T>::name;
    glz::reflect<T>{};
};


template <CtypesMappable T>
T *open_shm(std::string_view filepath, bool clear = false) {
    size_t size = sizeof(T);
    std::string path_str(filepath);
    int fd = open(path_str.c_str(), O_RDWR | O_CREAT, 0660);
    if (fd < 0) {
        throw std::runtime_error(std::format("Couldn't open the stats file: {}", filepath));
    }

    if (ftruncate(fd, size) == -1) {
        close(fd);
        throw std::runtime_error(
            std::format("Couldn't set the size of the stats file: {}", filepath));
    }

    void *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap");
        close(fd);
        throw std::runtime_error("mmap failed for: " + std::string(filepath));
    }
    if (clear) {
        // Manually clear the mapped memory
        memset(mapped, 0, size);
    }
    close(fd);
    return static_cast<T *>(mapped);
}

template <typename T>
void generate_python_binding(std::string_view ffi_path){
    std::vector<std::string> deps;
    std::ostringstream temp_out;
    generate_python_binding<T>(temp_out, deps);
    auto class_name = glz::meta<T>::name;
    auto path = std::format("{}/_{}.py", ffi_path, class_name);
    std::ofstream py_out(path);
    for (const auto &dep : deps) {
        py_out << std::format("from {}._{} import {}", ffi_path, dep, dep) << "\n";
    }
    py_out << "\n";
    py_out << temp_out.str();
    py_out.flush();
    py_out.close();
}




struct PairInfo {
    char base[16];
    char quote[16];
    char symbol[32];
    double price_precision;
    double quantity_precision;
};

struct Position{
    int side;
    double quantity;
    double price;
};

struct TradingSession{
    PairInfo pair_info;
    Position positions[10];
};

template <> struct glz::meta<PairInfo> {
    static constexpr std::string_view name = "PairInfo";
    using T = PairInfo;
    static constexpr auto value = object(
        "base", &T::base,
        "quote", &T::quote,
        "symbol", &T::symbol,
        "price_precision", &T::price_precision,
        "quantity_precision", &T::quantity_precision
    );
};


template <> struct glz::meta<Position> {
    static constexpr std::string_view name = "Position";
    using T = Position;
    static constexpr auto value = object(
        "side", &T::side,
        "quantity", &T::quantity,
        "price", &T::price
    );
};

template <> struct glz::meta<TradingSession> {
    static constexpr std::string_view name = "TradingSession";
    using T = TradingSession;
    static constexpr auto value = object(
        "pair_info", &T::pair_info,
        "positions", &T::positions
    );
};



int main() {

    namespace fs = std::filesystem;

    std::string_view output_dir = "build/ffi";
    fs::create_directories(output_dir);
    auto init_mod_path = std::format("{}/__init__.py", output_dir);
    if (!fs::exists(init_mod_path)) {
        std::ofstream init_file(init_mod_path);
        init_file.close();
    }

    generate_python_binding<Position>(output_dir);
    generate_python_binding<PairInfo>(output_dir);
    generate_python_binding<TradingSession>(output_dir);

    auto shared_data = open_shm<PairInfo>("build/data.bin", true);
    std::strncpy(shared_data->base, "BTC", 16);
    std::strncpy(shared_data->quote, "USD", 16);
    std::strncpy(shared_data->symbol, "BTC/USD", 32);
    shared_data->price_precision = 1;
    shared_data->quantity_precision = 0.0001;

    return 0;
}
