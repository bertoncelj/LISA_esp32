from datetime import datetime
from flask import Flask, request

app = Flask(__name__)

@app.route("/")
def hello():
    return "Hello World!"
    
    
    
@app.route('/', methods=['POST'])
def parse_request():
    print(request.data)  # data is empty
    # need posted data here

    with open('log.txt', 'a') as the_file:
        the_file.write(str(datetime.now()))
        the_file.write(" ")
        the_file.write(str(request.data, 'utf-8'))
        the_file.write("\n")
    return "Thanks!"

    
    
app.run(host='0.0.0.0', port=5000, debug=True)
