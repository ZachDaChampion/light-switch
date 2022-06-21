import "dotenv/config";
import axios from "axios";
import { Gpio } from "onoff";

const btn = new Gpio(17, "in", "rising", { debounceTimeout: 10 });

// Data to send to the plug
const req_data = {
  turn_off: '{"system":{"set_relay_state":{"state":0}}}',
  turn_on: '{"system":{"set_relay_state":{"state":1}}}',
  get_state: '{"system":{"get_sysinfo":{}}}',
};

// Send a request to the plug
function send_request(req_type: string) {
  return axios.post(
    `https://wap.tplinkcloud.com?token=${process.env.TPLINK_TOKEN}`,
    {
      method: "passthrough",
      params: {
        deviceId: process.env.TPLINK_DEVICE_ID,
        requestData: req_type,
      },
    }
  );
}

// Toggle the plug
async function toggle_plug() {
  // Get the current state of the device
  const state_resp = await send_request(req_data.get_state);
  const state_resp_data = JSON.parse(state_resp.data.result.responseData);
  const relay_state = state_resp_data.system.get_sysinfo.relay_state;

  // Toggle the device
  return send_request(relay_state ? req_data.turn_off : req_data.turn_on);
}

// Watch for button presses
btn.watch((err, value) => {
  if (err) {
    console.error(err);
    return;
  }
  console.log(`Button pressed: ${value}`);
  toggle_plug();
});

function cleanup() {
  btn.unwatch();
  btn.unexport();
}

process.on("SIGINT", cleanup);
