from flask import Flask, request, jsonify

app = Flask(__name__)
sensor_data = {}  # store latest data per device

@app.route('/sensor', methods=['POST'])
def sensor():
    data = request.get_json()
    device = data.get("device")
    sensor_data[device] = data
    print(f"Received from {device}: {data}")
    return jsonify({"status": "ok"})

@app.route('/sensor/<device>', methods=['GET'])
def get_device(device):
    return jsonify(sensor_data.get(device, {}))

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=40015)
