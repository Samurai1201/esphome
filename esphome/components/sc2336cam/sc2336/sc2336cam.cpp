#include "sc2336cam.h"
#include "esphome/core/log.h"

// Espressif components
#include "esp_cam_sensor.h"     // sensor descriptors, SC2336 driver
#include "esp_video.h"          // device/stream/pipeline APIs

namespace esphome {
namespace sc2336_cam {

static const char *const TAG = "sc2336cam";

void SC2336Cam::setup() {
  // 1) Init esp_video
  esp_video_config_t vcfg = {};
  vcfg.log_level = ESP_LOG_WARN;
  ESP_ERROR_CHECK(esp_video_init(&vcfg));

  // 2) Open the CSI camera device using SC2336 driver
  esp_video_device_open_config_t open_cfg = {};
  open_cfg.type = ESP_VIDEO_DEVICE_TYPE_CAMERA;
  open_cfg.camera.sensor_model = "SC2336";    // driver name from esp_cam_sensor
  open_cfg.camera.width  = this->width_;
  open_cfg.camera.height = this->height_;
  open_cfg.camera.fps    = this->fps_;
  open_cfg.camera.mipi_csi.lanes = 2;         // typical SC2336 MIPI module
  open_cfg.camera.mipi_csi.clock_mhz = 400;   // tune for your module

  // Enable ISP/JPEG so ESPHome can consume JPEG frames
  open_cfg.camera.format = ESP_VIDEO_PIXEL_FORMAT_JPEG;  // requires P4 ISP/JPEG
  open_cfg.camera.use_isp = this->use_isp_;

  // NOTE: If your board needs custom IPA JSON, use Kconfig option for SC2336
  // (CAMERA_SC2336_CUSTOMIZED_IPA_JSON_CONFIGURATION_FILE_PATH)
  // and place the file in your project root.
  // See Kconfig.sc2336 in esp_cam_sensor.
  // (We keep defaults here.)
  auto err = esp_video_device_open(&open_cfg, &this->video_dev_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "esp_video_device_open failed: %d", err);
    mark_failed();
    return;
  }

  // 3) Start the capture stream
  esp_video_stream_open_config_t scfg = {};
  scfg.type = ESP_VIDEO_STREAM_TYPE_CAPTURE;
  scfg.capture.buffer_count = 3;
  scfg.capture.timeout_ms = 1000;
  err = esp_video_stream_open(this->video_dev_, &scfg, &this->stream_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "esp_video_stream_open failed: %d", err);
    mark_failed();
    return;
  }

  ESP_LOGI(TAG, "SC2336 camera initialized: %dx%d@%dfps", width_, height_, fps_);
}

void SC2336Cam::dump_config() {
  ESP_LOGCONFIG(TAG, "SC2336 (MIPI-CSI) Camera");
  ESP_LOGCONFIG(TAG, "  Resolution: %dx%d", width_, height_);
  ESP_LOGCONFIG(TAG, "  FPS: %d", fps_);
  ESP_LOGCONFIG(TAG, "  ISP/JPEG: %s", use_isp_ ? "enabled" : "disabled");
}

size_t SC2336Cam::frame_data(uint8_t **data, size_t max_len) {
  if (!this->stream_) return 0;

  esp_video_frame_t frame = {};
  auto err = esp_video_stream_read(this->stream_, &frame, 100);  // 100 ms
  if (err != ESP_OK) return 0;

  // Expecting JPEG from ISP; pass directly to ESPHome
  // You could add a fallback to software JPEG if needed.
  *data = reinterpret_cast<uint8_t *>(frame.data);
  size_t len = frame.len;

  // If ESPHome copies the data immediately, we can return and
  // release frame back to the driver; otherwise keep a cache.
  // For simplicity, we just return and release:
  esp_video_stream_release(this->stream_, &frame);
  return len;
}

}  // namespace sc2336_cam
}  // namespace esphome
