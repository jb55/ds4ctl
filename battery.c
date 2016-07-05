
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glob.h>

struct controller {
  int r, g, b;
  int capacity;
  char uniq[20];
};

static char err[256];

int color_path(const char *device, const char *color, char *path) {
  glob_t pglob;
  static char filename[256];

  sprintf(filename, "%s/device/device/leds/*:%s", device, color);
  glob(filename, 0, 0, &pglob);

  if (pglob.gl_pathc == 0) {
    sprintf(err, "failed to glob '%s'", filename);
    globfree(&pglob);
    return 0;
  }

  if (pglob.gl_pathc > 1) {
    sprintf(err, "got more than one glob in '%s'", filename);
    globfree(&pglob);
    return 0;
  }

  strcpy(path, pglob.gl_pathv[0]);
  globfree(&pglob);

  return 1;
}

int write_color(const char *device, const char *color, int val) {
  FILE *f;
  static char path[256];
  static char filename[256];
  if (!color_path(device, color, path)) return 0;
  sprintf(filename, "%s/brightness", path);

  f = fopen(filename, "w+");

  if (!f) {
    sprintf(err, "could not open '%s'", filename);
    return 0;
  }

  if (!fprintf(f, "%d", val)) {
    sprintf(err, "could not write brightness value to file '%s'", filename);
    fclose(f);
    return 0;
  }

  fclose(f);
  return 1;
}

int read_color(const char *device, const char *color, int *val) {
  FILE *f;
  static char path[256];
  static char filename[256];
  if (!color_path(device, color, path)) return 0;
  sprintf(filename, "%s/brightness", path);

  f = fopen(filename, "r");

  if (!f) {
    sprintf(err, "could not open '%s'", filename);
    return 0;
  }

  if (!fscanf(f, "%d", val)) {
    sprintf(err, "could not read brightness value from file '%s'", filename);
    fclose(f);
    return 0;
  }

  fclose(f);
  return 1;
}

int write_trigger(const char *device, const char *trigger) {
  FILE *f;
  static char path[256];
  static char filename[256];
  if (!color_path(device, "global", path)) return 0;
  sprintf(filename, "%s/trigger", path);

  f = fopen(filename, "w+");
  if (!f) {
    sprintf(err, "could not open '%s'", filename);
    return 0;
  }

  fprintf(f, trigger);

  fclose(f);

  return 1;
}

int read_capacity(const char *device, int *capacity) {
  FILE *f;
  static char filename[256];
  glob_t pglob;

  sprintf(filename, "%s/device/device/power_supply/*/capacity", device);
  glob(filename, 0, 0, &pglob);

  if (pglob.gl_pathc == 0) {
    sprintf(err, "failed to glob '%s'", filename);
    globfree(&pglob);
    return 0;
  }

  if (pglob.gl_pathc > 1) {
    sprintf(err, "got more than one glob in '%s'", filename);
    globfree(&pglob);
    return 0;
  }

  strcpy(filename, pglob.gl_pathv[0]);
  globfree(&pglob);

  f = fopen(filename, "r");

  if (!f) {
    sprintf(err, "could not open '%s'", filename);
    return 0;
  }

  if (!fscanf(f, "%d", capacity)) {
    sprintf(err, "could not read capacity from '%s'", filename);
    fclose(f);
    return 0;
  }

  fclose(f);
  return 1;
}

int read_controller(const char *device, struct controller *controller) {
  if (!read_capacity(device, &controller->capacity)) return 0;
  if (!read_color(device, "red",   &controller->r)) return 0;
  if (!read_color(device, "green", &controller->g)) return 0;
  if (!read_color(device, "blue",  &controller->b)) return 0;
  return 1;
}

void fatal() {
  printf("error: %s\n", err);
  exit(1);
}

int main(int argc, char* argv[]) {
  struct controller controller;
  glob_t pglob;

  char *device;
  glob("/sys/class/input/js*", 0, 0, &pglob);

  if (pglob.gl_pathc == 0) {
    sprintf(err, "Could not enumerate any input devices");
    fatal();
  }

  for (size_t i = 0; i < pglob.gl_pathc; ++i) {
    device = pglob.gl_pathv[i];
    if (!read_controller(device, &controller)) {
      fatal();
    }
    else {
      /* controller.capacity = 20; */
      float percent = controller.capacity / 100.f;
      printf("\ndevice: %s\n", device);
      printf("capacity: %d%\n", controller.capacity);
      float scale = 255 * percent;
      printf("writing %f\n", scale);

      if (controller.capacity <= 10) {
        write_trigger(device, "heartbeat");
      }
      else {
        write_trigger(device, "none");
        write_color(device, "global", 1);
      }

      if (controller.capacity <= 25) {
        if (!write_color(device, "red", 10)) fatal();
        if (!write_color(device, "green", 0)) fatal();
        if (!write_color(device, "blue", 0)) fatal();
      }
      else if (controller.capacity <= 50) {
        if (!write_color(device, "red", 10)) fatal();
        if (!write_color(device, "green", 5)) fatal();
        if (!write_color(device, "blue", 0)) fatal();
      }
      else if (controller.capacity <= 75 ) {
        if (!write_color(device, "red", 10)) fatal();
        if (!write_color(device, "green", 10)) fatal();
        if (!write_color(device, "blue", 0)) fatal();
      }
      else if (controller.capacity <= 100 ) {
        if (!write_color(device, "red", 0)) fatal();
        if (!write_color(device, "green", 10)) fatal();
        if (!write_color(device, "blue", 0)) fatal();
      }
    }
  }

  globfree(&pglob);

  return 0;

}
