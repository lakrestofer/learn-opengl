
#include <cglm/cglm.h>
#include <cglm/mat4.h>
#include <cgltf/cgltf.h>
#define CGLTF_IMPLEMENTATION
#include "models/model.h"
#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#include "external/stb_image.h"

int format_to_gl_const(ImageFormat format) {
  switch (format) {
  case GRAY: return GL_LUMINANCE;
  case GRAY_ALPHA: return GL_LUMINANCE_ALPHA;
  case RED_GREEN_BLUE: return GL_RGB;
  case RED_GEEN_BLUE_ALPHA: return GL_RGBA;
  default: return -1;
  }
}

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

  model->n_materials = gltf_data->materials_count;
  model->materials   = calloc(model->n_materials, sizeof(Material));

  // load materials
  for (cgltf_size material_index = 0;
       material_index < gltf_data->materials_count;
       material_index++) {
    cgltf_material* m  = &(gltf_data->materials[material_index]);
    Material* material = &(model->materials[material_index]);

    // we only handle PBR metallic / roughness flow
    if (m->has_pbr_metallic_roughness) {

      cgltf_pbr_metallic_roughness pbr = m->pbr_metallic_roughness;

      material->textures[BASE] = (Texture){0};

      cgltf_texture* t      = NULL;
      cgltf_image* i        = NULL;
      cgltf_buffer_view* bv = NULL;
      cgltf_buffer* b       = NULL;

      // does this texture contain image data in a buffer?
      if (!(t = pbr.base_color_texture.texture) || !(i = t->image) ||
          !(bv = i->buffer_view) || !(b = bv->buffer) || !(b->data)) {
        continue;
      }

      // read data from buffer
      unsigned char* temp = malloc(bv->size);
      int o               = (int)bv->offset;
      int s               = (int)bv->stride ? (int)bv->stride : 1;
      for (unsigned int j = 0; j < bv->size; j++, o += s)
        temp[j] = ((unsigned char*)b->data)[o];

      // then parse that data
      ImageData image = {0};
      int n_channels  = 0;
      image.data      = stbi_load_from_memory(
          temp, bv->size, &image.w, &image.h, &n_channels, 0
      );
      if (0 <= n_channels && n_channels < N_IMAGE_FORMATS)
        image.format = n_channels;
      if (image.data)
        model->materials[material_index].textures[BASE].image = image;
    }
  }

  // load meshes
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

    printf("world transform\n");
    for (int i = 0; i < 4; i++) {
      printf("[");
      for (int j = 0; j < 4; j++) printf("%f, ", trans[i * 4 + j]);
      printf("]\n");
    }
    printf("\n");

    printf("world norm transform\n");
    for (int i = 0; i < 4; i++) {
      printf("[");
      for (int j = 0; j < 4; j++) printf("%f, ", trans_norm[i * 4 + j]);
      printf("]\n");
    }
    printf("\n");

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
        // number of bytes per element
        cgltf_size n_vecs     = accessor->count;
        cgltf_size floatCount = cgltf_accessor_unpack_floats(accessor, NULL, 0);
        float* data           = (float*)calloc(floatCount, sizeof(float));
        cgltf_accessor_unpack_floats(accessor, data, floatCount);

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
            glm_mat4_mulv3((vec4*)trans_norm, &data[3 * i], 1, &data[3 * i]);
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
        cgltf_type accessor_type  = accessor->type;
        cgltf_component_type type = accessor->component_type;

        // element size is should be same as component size
        assert(accessor_type == cgltf_type_scalar);

        cgltf_size n_elements = accessor->count;
        cgltf_size size       = cgltf_component_size(type);
        cgltf_size count =
            cgltf_accessor_unpack_indices(accessor, NULL, size, 0);
        assert(n_elements == count);
        void* buffer = calloc(count, size);
        cgltf_accessor_unpack_indices(accessor, buffer, size, count);

        mesh->n_triangles = n_elements / 3;

        // number of components
        if (type == cgltf_component_type_r_16u) {
          unsigned int* tmp = calloc(count, sizeof(unsigned int));
          for (cgltf_size i = 0; i < n_elements; i++)
            tmp[i] = ((unsigned short*)buffer)[i];
          free(buffer);
          mesh->indices = tmp;
        } else if (type == cgltf_component_type_r_32u) {
          mesh->indices = buffer;
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
