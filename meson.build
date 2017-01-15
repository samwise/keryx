project('tutorial','cpp')

# internal dependencies
lib=shared_library('mylib','MathFunctions/mysqrt.cpp')

# external dependencies
zdep = dependency('zlib', version : '>=1.2.8')

executable('tutorial',
           'tutorial.cpp', dependencies: zdep,
           link_with: lib)