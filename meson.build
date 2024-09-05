project('learn-opengl', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])


glfw_proj = subproject('glfw', default_options: ['default_library=static'])
glfw_dep = glfw_proj.get_variable('glfw_dep')

deps = [
  glfw_dep
]

src = [
  './src/main.c',
  './src/gl.c'
]
incdir = include_directories('include')
exe = executable(
  'learn-opengl',
  sources: src,
  include_directories : incdir,
  dependencies: deps,
  install : false
)
