project('tutorial','cpp')

lib=shared_library('mylib','MathFunctions/mysqrt.cpp')

zdep = dependency('zlib', version : '>=1.2.8')

executable('tutorial',
           'tutorial.cpp',
	   dependencies: zdep,
           link_with: lib)