from flask import Flask, request
import csv

app = Flask(__name__)
csv_file = 'current_data.csv'


@app.route('/data', methods=['POST'])
def receive_data():
    data = request.json
    if not data:
        return 'Invalid data', 400

    current = data.get('current')
    timestamp = data.get('timestamp')

    with open(csv_file, 'a', newline='') as f:
        writer = csv.writer(f)
        writer.writerow([timestamp, current])

    return 'OK', 200


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
