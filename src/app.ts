import "dotenv/config";
import axios from "axios";

const req_data = {
  turn_off: '{"system":{"set_relay_state":{"state":0}}}',
  turn_on: '{"system":{"set_relay_state":{"state":1}}}',
  get_state: '{"system":{"get_sysinfo":{}}}',
};

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

// Get the current state of the device
const state_resp = await send_request(req_data.get_state);
const state_resp_data = JSON.parse(state_resp.data.result.responseData);
const relay_state = state_resp_data.system.get_sysinfo.relay_state;

// Toggle the device
send_request(relay_state ? req_data.turn_off : req_data.turn_on);
