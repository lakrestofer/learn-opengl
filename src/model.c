
#include <cglm/cglm.h>
#include <cglm/mat4.h>
#define CGLTF_IMPLEMENTATION
#include "models/model.h"
#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>

void printGltfInfo(cgltf_data* data) {
  printf("> stats:\n");
  printf("> - n scenes: %zu\n", data->scenes_count);
  printf("> - n nodes: %zu\n", data->nodes_count);
  printf("> - n meshes : %zu\n", data->meshes_count);
  printf("> - n materials : %zu\n", data->materials_count);
  printf("> - n accessors: %zu\n", data->accessors_count);
  printf("> - n buffer_views: %zu\n", data->buffer_views_count);
  printf("> - n buffers : %zu\n", data->buffers_count);
  printf("> - n images : %zu\n", data->images_count);
  printf("> - n textures: %zu\n", data->textures_count);
  printf("> - n samplers: %zu\n", data->samplers_count);
  printf("\n");
}

/// helping macro that takes the information provided by the accessor
/// and copies the data to a destination buffer
#define LOAD_ATTRIBUTE(accessor, numComp, dataType, dstPtr)                    \
  {                                                                            \
    int n            = 0;                                                      \
    dataType* buffer = (dataType*)accessor->buffer_view->buffer->data +        \
                       accessor->buffer_view->offset / sizeof(dataType) +      \
                       accessor->offset / sizeof(dataType);                    \
    for (unsigned int k = 0; k < accessor->count; k++) {                       \
      for (int l = 0; l < numComp; l++) {                                      \
        dstPtr[numComp * k + l] = buffer[n + l];                               \
      }                                                                        \
      n += (int)(accessor->stride / sizeof(dataType));                         \
    }                                                                          \
  }

// NOTE: this function very closely mimics LoadGLTF from raylib
LoadModelRes loadModelFromGltfFile(const char* path, Model* model) {
  printf("MODEL: Loading gltf (glb) file: (%s)\n", path);
  cgltf_options ops     = {0};
  cgltf_data* gltf_data = NULL;
  cgltf_result gltf_err = cgltf_parse_file(&ops, path, &gltf_data);
  if (gltf_err) return ERROR; // could not load gltf data
  gltf_err = cgltf_validate(gltf_data);
  if (gltf_err) return ERROR ; // data was loaded but not valid!
  gltf_err = cgltf_load_buffers(&ops, gltf_data, path);
  if (gltf_err) return ERROR; // could not load data into buffers (NOTE: I'm a bit unsure what this means in the context of gdb files)
  // we don't support multi scene models
  if (gltf_data->scenes_count != 1) return ERROR;
  if (gltf_data->file_type != cgltf_file_type_glb) return ERROR; // only glb
  printf("> successfully loaded gltf data\n");
  printGltfInfo(gltf_data);

  // we iterate through the tree of nodes
  // and gather up all meshes.

  // first we count them such that we can allocate enough space
  // NOTE: each 'primitive' in each gltf mech will correspond
  // to a Mesh
  int n_meshes = 0;
  for (cgltf_size ni = 0; ni < gltf_data->nodes_count; ni++) {
    cgltf_node* n = &(gltf_data->nodes[ni]);
    cgltf_mesh* m = n->mesh;
    if (!m) continue;
    for (cgltf_size pi = 0; pi < n->mesh->primitives_count; pi++)
      if (m->primitives[pi].type == cgltf_primitive_type_triangles) n_meshes++;
  }
  printf("> loading n=%d meshes!\n", n_meshes);

  model->n_meshes = n_meshes;
  model->meshes   = calloc(n_meshes, sizeof(Mesh));

  int mesh_index = 0; // the location of the currently being constructed mesh
  // and then we actually gather them up
  for (cgltf_size ni = 0; ni < gltf_data->nodes_count; ni++) {
    cgltf_node* node = &(gltf_data->nodes[ni]); // a node is a single 'object'
    printf("> proccessing node %s\n", node->name);
    cgltf_mesh* mesh = node->mesh;              // which is contains one mesh

    // the node might define some transform.
    // moving each mesh to make the model as a whole.
    cgltf_float world_transform[16];
    cgltf_node_transform_world(node, world_transform);
    cgltf_float world_transform_normal[16];
    glm_mat4_inv((vec4*) world_transform, (vec4*)world_transform_normal);
    glm_mat4_transpose((vec4*)world_transform_normal);
    

    // the mesh contains several primitives
    // which defines its vertices, normals, tangents etc.
    // as done in raylib, we will create a Mesh for each such primitive
    for (cgltf_size pi = 0; pi < mesh->primitives_count; pi++) {
      cgltf_primitive* primitive = &(mesh->primitives[pi]);
      printf("> proccessing primitive %zu\n", pi);
      // we don't support non triangular meshes
      if (primitive->type != cgltf_primitive_type_triangles) continue;

      // each primitive has serveral attributes, which descibe each
      // primitive's vertices, normals, tangents etc.
      for (cgltf_size ai = 0; ai < primitive->attributes_count; ai++) {
        cgltf_attribute* attribute          = &(primitive->attributes[ai]);
        cgltf_accessor* accessor            = attribute->data;
        cgltf_attribute_type attribute_type = attribute->type;
        cgltf_type accessor_type            = accessor->type;
        cgltf_component_type component_type = accessor->component_type;

        printf("> proccessing primitive %zu, attribute %zu - %s\n", pi, ai, attribute->name);

#define IS_PRIMITIVE(attribute, accessor, component) ((attribute_type == cgltf_attribute_type_##attribute) && (accessor_type == cgltf_type_##accessor) && (component_type == cgltf_component_type_##component))

        if (IS_PRIMITIVE(position, vec3, r_32f)) {
          // primitive is array of vertex vectors (float[3])
          model->meshes[mesh_index].n_vertices = (int)accessor->count;
          model->meshes[mesh_index].vertices =
              malloc(accessor->count * 3 * sizeof(float));

          LOAD_ATTRIBUTE(
              accessor, 3, float, model->meshes[mesh_index].vertices
          );

          float* vertices = model->meshes[mesh_index].vertices;
          for (cgltf_size vi = 0; vi < accessor->count; vi++) {
            // transform
            glm_mat4_mulv3(
                (vec4*)world_transform, &vertices[3 * vi], 1, &vertices[3 * vi]
            );
          }

        } else if (IS_PRIMITIVE(normal, vec3, r_32f)) {
          // primitive is array of normal vectors (float[3])
        } else if (IS_PRIMITIVE(tangent, vec4, r_32f)) {
          // primitive is array of tangent vectors (float[4])
        } else if (IS_PRIMITIVE(texcoord, vec2, r_32f)) {
          // primitive is array of texture coordinate vectors (float[2])
        } else {
          printf("> primitive is unsupported array\n");
          printf("> attribute type: %d, accessor type: %d, component type: %d\n", attribute_type, accessor_type, component_type);
          // unsupported!
        }
#undef IS_PRIMITIVE
      }
    }
  }

// clean:
  if (gltf_data) cgltf_free(gltf_data);

  return SUCCESS;
}

void freeMesh(Mesh* mesh) {
  if (mesh->vertices) free(mesh->vertices);
  if (mesh->indices) free(mesh->indices);
}

void freeModel(Model* model) {
  for (int mi = 0; mi < model->n_meshes; mi++) freeMesh(&model->meshes[mi]);
  free(model->meshes);
}
