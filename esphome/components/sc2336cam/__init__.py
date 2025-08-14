import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import camera
from esphome.const import CONF_ID

AUTO_LOAD = ["camera"]

sc2336_cam_ns = cg.esphome_ns.namespace("sc2336cam")
SC2336Cam = sc2336_cam_ns.class_("SC2336Cam", camera.Camera, cg.Component)

CONFIG_SCHEMA = camera.CAMERA_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(SC2336Cam),
    cv.Optional("width", default=1920): cv.int_,
    cv.Optional("height", default=1080): cv.int_,
    cv.Optional("fps", default=15): cv.int_,
    cv.Optional("use_isp", default=True): cv.boolean,
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await camera.register_camera(var, config)
    cg.add(var.set_width(config["width"]))
    cg.add(var.set_height(config["height"]))
    cg.add(var.set_framerate(config["fps"]))
    cg.add(var.set_use_isp(config["use_isp"]))
