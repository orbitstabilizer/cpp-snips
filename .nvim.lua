vim.print("Loading .nvim.lua")



M = {
    cur_program = "",
    makeprg = "",
    auto_relaod = false,
}
M.set_program = function(path)
    M.cur_program = path
    M.auto_relaod = false
end

M.run = function()
    if M.cur_program == "" then
        print("No program set. Use :SetProgram <path_to_executable> to set the program to run.")
        return
    end
    vim.cmd("! " .. M.cur_program)
end

M.disable_auto_reload = function()
    M.auto_relaod = false
    if vim.fn.exists("#NvimSession#BufWritePost") == 1 then
        vim.api.nvim_del_augroup_by_name("NvimSession")
    end
end

M.enable_auto_reload = function()
    M.auto_relaod = true
    vim.api.nvim_create_autocmd("BufWritePost", {
        group = vim.api.nvim_create_augroup("NvimSession", { clear = true }),
        pattern = ".nvim.lua",
        callback = M.reload,
    })
end

M.reload = function()
    dofile(vim.fn.expand("%"))
end


M.setup = function(opts)
    M.auto_relaod = opts.auto_reload or false
    M.makeprg = opts.makeprg or M.makeprg
    M.cur_program = opts.cur_program or M.cur_program

    if M.makeprg ~= "" then
        vim.opt.makeprg = M.makeprg
    end

    vim.api.nvim_create_user_command("SetProgram", function(_opts)
        M.set_program(_opts.args)
    end, { nargs = 1 })

    vim.api.nvim_create_user_command("Run", function()
        M.run()
    end, {})

    vim.api.nvim_create_user_command("BuildAndRun", function()
        if M.makeprg == "" then
            print("No makeprg set. Use :SetMakeprg <command> to set the build command.")
            return
        end
        vim.cmd("make")
        M.run()
    end, {})

    vim.api.nvim_create_user_command("SessionToggleAutoReload", function()
        if M.auto_relaod then
            M.disable_auto_reload()
            print("Auto reload disabled.")
        else
            M.enable_auto_reload()
            print("Auto reload enabled.")
        end
    end, {})

    vim.api.nvim_create_user_command("SessionReload", function()
        M.reload()
        print("Configuration reloaded.")
    end, {})


    vim.api.nvim_set_keymap("n", "<leader>r", ":Run<CR>", { noremap = true, silent = true })

    if M.auto_relaod then
        M.enable_auto_reload()
    end
end

M.setup({
    auto_reload = true,
    makeprg = "cmake -B build && cmake --build build -j8",
    cur_program = "./build/hello-world",
})

