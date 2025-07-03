from flask import Flask, jsonify, render_template, request, redirect, session
import requests

app = Flask(__name__)

app.secret_key = 'yoursecret'  # required for session

openWeatherAPIKEY = 'APIKEY'

ESP32_IP = "http://192.168.2.236"  # Change to your ESP32 IP

@app.route('/')
def home():
    esp_ip = session.get('esp_ip', 'http://192.168.2.236')
    location = session.get('location', 'Halifax')  # <-- define location here
    try:
        res = requests.get(f"{esp_ip}/data", timeout=5)
        data = res.json()
    except:
        data = {"time": "-", "temp": "-", "hum": "-", "light": "-", "water": "-", "soil": "-"}


    return render_template("home.html", data=data, location=location)

@app.route('/set-location', methods=['POST'])
def set_location():
    session['location'] = request.form['location']
    return redirect('/')

@app.route('/set-ip', methods=['POST'])
def set_ip():
    ip = request.form['esp_ip']
    session['esp_ip'] = ip
    return redirect('/')

@app.route("/trends")
def trends():
    return render_template("trends.html")

@app.route("/info")
def info():
    return render_template("info.html")

@app.route('/toggle')
def toggle():
    try:
        requests.get(f"{session.get('esp_ip', 'http://192.168.2.236')}/toggle", timeout=5)
    except:
        pass
    return redirect('/')

@app.route('/log')
def get_log():
    try:
        esp_response = requests.get(f"{session.get('esp_ip', 'http://192.168.2.236')}/log", timeout=5)
        with open("log.csv", "w") as f:
            f.write(esp_response.text)
        return esp_response.text
    except:
        return "Failed to fetch log from ESP32", 500


@app.route("/data")
def get_data():
    try:
        res = requests.get(f"{session.get('esp_ip', 'http://192.168.2.236')}/data", timeout=5)
        data = res.json()
    except FileNotFoundError:
        return "Log file not found", 404
    return jsonify(data)

@app.route('/set-constraints', methods=['POST'])
def set_constraints():
    temp_min = request.form['tempMin']
    temp_max = request.form['tempMax']
    try:
        requests.get(f"{ESP32_IP}/set-constraints?min={temp_min}&max={temp_max}", timeout=5)
    except:
        pass
    return redirect('/')





if __name__ == '__main__':
    app.run(debug=True)
