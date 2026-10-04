from flask import Flask, jsonify, request, send_from_directory
import os

app = Flask(__name__)


# =====================================================
# TOURIST DATA
# =====================================================

tourist_data = {
    "kashmir": {
        "history": {
            "title": "Kashmir History:",
            "text": "speaking..."
        },

        "tourist_spot": {
            "title": "Kashmir Tourist Spots(heritage sites and other touristy locations.):",
            "text": "speaking..."
        },

        "hidden_gem": {
            "title": "Kashmir Hidden Gem:",
            "text": "speaking..."
        },

        "food": {
            "title": "Kashmir Food",
            "text": "speaking..."
        }
    },
    "mumbai": {
        "history": {
            "title": "Mumbai History:",
            "text": "speaking..."
        },

        "tourist_spot": {
            "title": "Mumbai Tourist Spots(heritage sites and other touristy locations.):",
            "text": "speaking..."
        },

        "hidden_gem": {
            "title": "Mumbai Hidden Gem:",
            "text": "speaking..."
        },

        "food": {
            "title": "Mumbai Food",
            "text": "speaking..."
        }
    },
    "surat": {
        "history": {
            "title": "Surat History:",
            "text": "speaking..."
        },

        "tourist_spot": {
            "title": "Surat Tourist Spots(heritage sites and other touristy locations.):",
            "text": "speaking..."
        },

        "hidden_gem": {
            "title": "Surat Hidden Gem:",
            "text": "speaking..."
        },

        "food": {
            "title": "Surat Food",
            "text": "speaking..."
        }
    },
    "delhi": {
        "history": {
            "title": "Delhi History:",
            "text": "speaking..."
        },

        "tourist_spot": {
            "title": "Delhi Tourist Spots(heritage sites and other touristy locations.):",
            "text": "speaking..."
        },

        "hidden_gem": {
            "title": "Delhi Hidden Gem:",
            "text": "speaking..."
        },

        "food": {
            "title": "Delhi Food",
            "text": "speaking..."
        }
    },
    "jaipur": {
        "history": {
            "title": "Jaipur History:",
            "text": "speaking..."
        },

        "tourist_spot": {
            "title": "Jaipur Tourist Spots(heritage sites and other touristy locations.):",
            "text": "speaking..."
        },

        "hidden_gem": {
            "title": "Jaipur Hidden Gem:",
            "text": "speaking..."
        },

        "food": {
            "title": "Jaipur Food",
            "text": "speaking..."
        }
    },
    "meghalaya": {
        "history": {
            "title": "Meghalaya History:",
            "text": "speaking..."
        },

        "tourist_spot": {
            "title": "Meghalaya Tourist Spots(heritage sites and other touristy locations.):",
            "text": "speaking..."
        },

        "hidden_gem": {
            "title": "Meghalaya Hidden Gem:",
            "text": "speaking..."
        },

        "food": {
            "title": "Meghalaya Food",
            "text": "speaking..."
        }
    },
}


# =====================================================
# AUDIO FOLDER
# =====================================================

AUDIO_FOLDER = os.path.join(os.path.dirname(__file__), "audios")


# =====================================================
# HOME
# =====================================================

@app.route("/")
def home():

    return jsonify({
        "message": "Tourist Guide Backend is running!"
    })


# =====================================================
# LOCATION
# =====================================================

@app.route("/api/location", methods=["POST"])
def location():

    data = request.get_json()

    if not data:

        return jsonify({"error": "No JSON data received"}), 400

    latitude = data.get("latitude")
    longitude = data.get("longitude")

    print()
    print("GPS LOCATION")
    print("Latitude:", latitude)
    print("Longitude:", longitude)

    if latitude is None or longitude is None:

        return jsonify({"error": "Latitude or longitude missing"}), 400

    state = detect_state(latitude, longitude)

    return jsonify({
        "state": state,
        "latitude": latitude,
        "longitude": longitude
    })


# =====================================================
# STATE DETECTION
# =====================================================

def detect_state(latitude, longitude):

    if (32.28 <= latitude <= 36.97 and 73.43 <= longitude <= 83.50):
        return "kashmir"
    elif (28.40 <= latitude <= 28.88 and 76.84 <= longitude <= 77.35):
        return "delhi"
    elif (18.89 <= latitude <= 19.27 and 72.77 <= longitude <= 72.98):
        return "mumbai"
    elif (21.05 <= latitude <= 21.25 and 72.75 <= longitude <= 72.95):
        return "surat"
    elif (26.75 <= latitude <= 27.05 and 75.65 <= longitude <= 75.95):
        return "jaipur"
    elif (25.02 <= latitude <= 26.10 and 89.80 <= longitude <= 92.80):
        return "meghalaya"
    else:
        return "unknown"


# =====================================================
# TOURIST INFORMATION
# =====================================================

@app.route("/api/info/<state>/<category>", methods=["GET"])
def get_info(state, category):

    state = state.lower()
    category = category.lower()

    if state not in tourist_data:
        return jsonify({"error": "State not found"}), 404

    if category not in tourist_data[state]:
        return jsonify({"error": "Category not found"}), 404

    information = tourist_data[state][category]

    return jsonify({
        "state": state,
        "category": category,
        "title": information["title"],
        "text": information["text"]
    })


# =====================================================
# AUDIO
# =====================================================

@app.route("/api/audio/<state>/<category>", methods=["GET"])
def get_audio(state, category):

    state = state.lower()
    category = category.lower()

    allowed_states = ["kashmir", "delhi", "meghalaya", "surat", "mumbai", "jaipur"]
    allowed_categories = ["history", "tourist_spot", "hidden_gem", "food"]

    if state not in allowed_states:
        return jsonify({"error": "State not found"}), 404

    if category not in allowed_categories:
        return jsonify({"error": "Category not found"}), 404

    filename = (state + "_" + category + ".mp3")
    filepath = os.path.join(AUDIO_FOLDER, filename)

    if not os.path.isfile(filepath):
        return jsonify({
            "error": "Audio file not found",
            "expected_file": filename
        }), 404

    return send_from_directory(
        AUDIO_FOLDER,
        filename,
        mimetype="audio/mpeg"
    )


# =====================================================
# RUN SERVER
# =====================================================

if __name__ == "__main__":

    print()
    print("==============================")
    print(" TOURIST GUIDE BACKEND")
    print("==============================")

    print()
    print("Server running on:")
    print("http://0.0.0.0:5000")

    print()

    app.run(
        host="0.0.0.0",
        port=5000,
        debug=True
    )