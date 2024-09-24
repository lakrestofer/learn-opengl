project('learn-opengl', 'c',
  version : '0.1',
  default_options : [
    'builttype=debug',
    'warning_level=3',
    'c_std=c11'
  ]
)


# system dependencies
glfw_dep = dependency('glfw3')
cblas_dep = dependency('cblas')
cglm_dep = dependency('cglm')

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

deps = [
  glfw_dep,
  cblas_dep,
  cglm_dep,
  m_dep,
]

subdir('src')
incdir = include_directories('include')
exe = executable(
  'learn-opengl',
  sources: sources,
  include_directories : incdir,
  dependencies: deps,
  install : false
)
