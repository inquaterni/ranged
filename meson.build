project('ranged', 'cpp',
        version : '1.0.0',
        default_options : ['warning_level=3', 'cpp_std=c++11', 'buildtype=release'])

lib_args = ['-DBUILDING_MESON_LIBRARY']

libranged = shared_library(
        'ranged',
        install: false,
        cpp_args: lib_args,
        include_directories: include_directories('..', '../include')
)

subdir('tests')