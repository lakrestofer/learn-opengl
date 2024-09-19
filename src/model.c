#include "models/model.h"
#include <stdio.h>

void print_gltf_info(cgltf_data* data, const char* file_name) {

  printf("=== loaded glft model ===\n");
  printf("- file name: %s\n", file_name);
  printf("- n meshes : %zu\n", data->meshes_count);
  printf("- n materials : %zu\n", data->materials_count);
  printf("- n accessors: %zu\n", data->accessors_count);
  printf("- n buffer_views: %zu\n", data->buffer_views_count);
  printf("- n buffers : %zu\n", data->buffers_count);
  printf("- n images : %zu\n", data->images_count);
  printf("- n textures: %zu\n", data->textures_count);
  printf("- n samplers: %zu\n", data->samplers_count);
  printf("- n skins: %zu\n", data->skins_count);
  printf("- n cameras: %zu\n", data->cameras_count);
  printf("- n lights: %zu\n", data->lights_count);
  printf("- n nodes: %zu\n", data->nodes_count);
  printf("- n scenes: %zu\n", data->scenes_count);
  printf("- n animations: %zu\n", data->animations_count);
  printf("- n variants: %zu\n", data->variants_count);
  printf("- n data extensions: %zu\n", data->data_extensions_count);
  printf("- n extensions used: %zu\n", data->extensions_used_count);
  printf(
      "- n required extensions used: %zu\n", data->extensions_required_count
  );
  printf("- json size: %zu\n", data->json_size);
  printf("- bin size: %zu\n", data->bin_size);
}
