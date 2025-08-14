#pragma once
#include "esphome/components/camera/camera.h"
#include "esphome/core/component.h"

namespace esphome {
namespace sc2336cam {

class SC2336Cam : public camera::Camera, public Component {
 public:
  void set_width(int w) { width_ = w; }
  void set_height(int h) { height_ = h; }
  void set_framerate(int fps) { fps_ = fps; }
  void set_use_isp(bool en) { use_isp_ = en; }

  void setup() override;
  void dump_config() override;
  size_t frame_data(uint8_t **data, size_t max_len) override;
  int get_image_width() override { return width_; }
  int get_image_height() override { return height_; }
  bool get_vertical_flip() const override { return false; }
  bool get_horizontal_mirror() const override { return false; }

 protected:
  int width_{1920};
  int height_{1080};
  int fps_{15};
  bool use_isp_{true};

  // opaque handles to the video pipeline / device
  void *video_dev_{nullptr};
  void *stream_{nullptr};

  // simple buffer caching
  uint8_t *last_frame_{nullptr};
  size_t last_frame_len_{0};
};

}  // namespace sc2336_cam
}  // namespace esphome
