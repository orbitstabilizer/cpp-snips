import numpy as np
import mmap
import sys

sys.path.append('./build/')
from ffi._PairInfo import PairInfo
from ffi_utils import dtype_from_ctypes_type

with open('./build/data.bin', 'r+b') as f:
    mm = mmap.mmap(f.fileno(), 0)
    pair_info = PairInfo.from_buffer(mm)

    b = np.frombuffer(mm, dtype=dtype_from_ctypes_type(PairInfo))
    print(b)
