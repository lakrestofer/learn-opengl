
#include <cglm/cglm.h>
#include <cglm/mat4.h>
#define CGLTF_IMPLEMENTATION
#include "models/model.h"
#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>

/// helping macro that takes the information provided by the accessor
/// and copies the data to a destination buffer
#define LOAD_ATTRIBUTE(accessor, numComp, dataType, dstPtr)                    \
  {                                                                            \
    int _n            = 0;                                                     \
    dataType* _buffer = (dataType*)accessor->buffer_view->buffer->data +       \
                        accessor->buffer_view->offset / sizeof(dataType) +     \
                        accessor->offset / sizeof(dataType);                   \
    for (unsigned int k = 0; k < accessor->count; k++) {                       \
      for (int l = 0; l < numComp; l++) {                                      \
        dstPtr[numComp * k + l] = _buffer[_n + l];                             \
      }                                                                        \
      _n += (int)(accessor->stride / sizeof(dataType));                        \
    }                                                                          \
  }

/// macro for reducing the amount of visual clutter
/// NOTE: this macro is NOT hygienic
#define IS_PRIMITIVE(attr, accr, comp)                                         \
  ((attribute_type == cgltf_attribute_type_##attr) &&                          \
   (accessor_type == cgltf_type_##accr) &&                                     \
   (component_type == cgltf_component_type_##comp))

// NOTE: this function very closely mimics LoadGLTF from raylib
LoadModelRes loadModelFromGltfFile(const char* path, Model* model) {
  // load gltf file
  cgltf_options ops     = {0};
  cgltf_data* gltf_data = NULL;
  cgltf_result gltf_err = cgltf_parse_file(&ops, path, &gltf_data) ||
                          cgltf_validate(gltf_data) ||
                          cgltf_load_buffers(&ops, gltf_data, path);
  if (gltf_err) return ERROR;

  // check some simple constraints
  if (gltf_data->scenes_count != 1 ||
      gltf_data->file_type != cgltf_file_type_glb)
    return ERROR;

  // we want to extract all meshes from the file and transform them
  // into a format that is easier for us to manage.

  // first we count the meshes them such that we can allocate enough space
  int n_meshes = 0;
  for (cgltf_size ni = 0; ni < gltf_data->nodes_count; ni++) {
    cgltf_node* n = &(gltf_data->nodes[ni]);
    cgltf_mesh* m = n->mesh;
    if (!m) continue;
    for (cgltf_size pi = 0; pi < n->mesh->primitives_count; pi++) {
      if (m->primitives[pi].type == cgltf_primitive_type_triangles) n_meshes++;
    }
  }
  printf("> loading n=%d meshes!\n", n_meshes);

  model->n_meshes = n_meshes;
  model->meshes   = calloc(n_meshes, sizeof(Mesh));

  int mesh_index = 0; // the currently 'being constructed' mesh
  for (cgltf_size ni = 0; ni < gltf_data->nodes_count; ni++) {
    cgltf_node* gltf_node =
        &(gltf_data->nodes[ni]); // a node is a single 'object'
    printf("> processing node %s\n", gltf_node->name);
    cgltf_mesh* gltf_mesh = gltf_node->mesh; // which is contains one mesh

    // the node might define some transform.
    // moving each mesh to make the model as a whole.

    /// world transform
    cgltf_float trans[16];
    cgltf_node_transform_world(gltf_node, trans);
    cgltf_float trans_norm[16];
    glm_mat4_inv((vec4*)trans, (vec4*)trans_norm);
    glm_mat4_transpose((vec4*)trans_norm);

    // the mesh contains several primitives
    // which defines its vertices, normals, tangents etc.
    // as done in raylib, we will create a Mesh for each such primitive
    for (cgltf_size pi = 0; pi < gltf_mesh->primitives_count; pi++) {
      cgltf_primitive* primitive = &(gltf_mesh->primitives[pi]);
      printf("> processing primitive %zu\n", pi);

      // we don't support non triangular meshes
      if (primitive->type != cgltf_primitive_type_triangles) continue;

      // each primitive will correspond to one `Mesh`
      // we will fill out the fields of this mesh
      // instance by iterating through the attributes.
      // When we've successfully filled all existing
      // fields, we increase `mesh_index`
      Mesh* mesh = &model->meshes[mesh_index];

      // we start by loading vertices, tangents, normals, texture coordinates
      for (cgltf_size ai = 0; ai < primitive->attributes_count; ai++) {
        cgltf_attribute* attribute          = &(primitive->attributes[ai]);
        cgltf_accessor* accessor            = attribute->data;
        cgltf_attribute_type attribute_type = attribute->type;
        cgltf_type accessor_type            = accessor->type;
        cgltf_component_type component_type = accessor->component_type;

        printf(
            "> processing primitive %zu, attribute %zu - %s\n",
            pi,
            ai,
            attribute->name
        );

        // we can load the data up front
        // the accessor tells us how to extract the data from
        // the gltf buffers
        int n_floats      = cgltf_num_components(accessor->type);
        cgltf_size n_vecs = accessor->count;
        float* data       = malloc(n_vecs * n_floats * sizeof(float));
        LOAD_ATTRIBUTE(accessor, n_floats, float, data);

        // depending on attribute type, set the correct field
        if (IS_PRIMITIVE(position, vec3, r_32f)) {
          mesh->n_vertices = n_vecs;
          mesh->vertices   = data;
          for (cgltf_size i = 0; i < n_vecs; i++) {
            glm_mat4_mulv3((vec4*)trans, &data[3 * i], 1, &data[3 * i]);
          }
        } else if (IS_PRIMITIVE(normal, vec3, r_32f)) {
          mesh->normals = data;
          for (cgltf_size i = 0; i < n_vecs; i++) {
            glm_mat4_mulv3((vec4*)trans, &data[3 * i], 1, &data[3 * i]);
          }
        } else if (IS_PRIMITIVE(tangent, vec4, r_32f)) {
          mesh->tangents = data;
          for (cgltf_size i = 0; i < n_vecs; i++) {
            glm_mat4_mulv3((vec4*)trans, &data[3 * i], 1, &data[3 * i]);
          }
        } else if (IS_PRIMITIVE(texcoord, vec2, r_32f)) {
          // only support one texture per mesh for now
          if (attribute->index == 0) mesh->tex_coords = data;
        } else {
          printf("> primitive is unsupported array\n");
          printf(
              "> attribute type: %d, accessor type: %d, component type: %d\n",
              attribute_type,
              accessor_type,
              component_type
          );
          // unsupported!
        }
      }
      // then we check for and load indices
      if (primitive->indices) {
        printf("> processing primitive %zu, checking if indices exists\n", pi);
        cgltf_accessor* accessor  = primitive->indices;
        cgltf_component_type type = accessor->component_type;
        int n_indices             = accessor->count;
        mesh->n_triangles         = n_indices / 3;

        mesh->indices = malloc(n_indices * sizeof(unsigned short));
        if (type == cgltf_component_type_r_16u) {
          LOAD_ATTRIBUTE(accessor, 1, unsigned short, mesh->indices);
        } else if (type == cgltf_component_type_r_32u) {
          unsigned int* temp = malloc(n_indices * sizeof(unsigned int));
          LOAD_ATTRIBUTE(accessor, 1, unsigned int, temp);
          for (int i = 0; i < n_indices; i++) mesh->indices[i] = temp[i];
          free(temp);
        }
      } else {
        mesh->n_triangles = mesh->n_vertices / 3;
      }
      mesh_index++;
    }
  }

  // clean:
  if (gltf_data) cgltf_free(gltf_data);

  return SUCCESS;
}
#undef IS_PRIMITIVE

void freeMesh(Mesh* mesh) {
  if (mesh->vertices) free(mesh->vertices);
  if (mesh->normals) free(mesh->normals);
  if (mesh->tangents) free(mesh->tangents);
  if (mesh->tex_coords) free(mesh->tex_coords);
  if (mesh->indices) free(mesh->indices);
}

void freeModel(Model* model) {
  for (int mi = 0; mi < model->n_meshes; mi++) freeMesh(&model->meshes[mi]);
  free(model->meshes);
}
