project('learn-opengl', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])


# system dependencies
glfw_dep = dependency('glfw3')
cblas_dep = dependency('cblas')
cglm_dep = dependency('cglm')
assimp_dep = dependency('assimp')

deps = [
  glfw_dep,
  cblas_dep,
  assimp_dep
]

src = [
  './src/main.c',
  './src/gl.c',
  './src/core.c'
]
incdir = include_directories('include')
exe = executable(
  'learn-opengl',
  sources: src,
  include_directories : incdir,
  dependencies: deps,
  install : false
)
