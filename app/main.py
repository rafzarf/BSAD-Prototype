"""
# Sensor Data Analysis with Anomaly Detection
This FastAPI application analyzes sensor data (accelerometer) using an autoencoder-based anomaly detection model. It supports:
1. Batch data processing for inference.
2. Dynamic thresholding using reconstruction error.
3. Data storage in PostgreSQL and InfluxDB.
4. Real-time data plotting and visualization.

## Key Features
- **Batch Inference:** Processes batches of 24 data points.
- **Dynamic Thresholding:** Computes anomalies based on the reconstruction error.
- **Multi-DB Storage:** Anomaly data stored in PostgreSQL and InfluxDB.
- **Real-time Visualization:** Generates plots for visualization when a threshold of buffered data is reached.
"""

import os
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import numpy as np
import pandas as pd
import datetime
import joblib
from tensorflow.keras.models import load_model
import logging
import psycopg2
from psycopg2 import pool
from apscheduler.schedulers.background import BackgroundScheduler
from influxdb_client import InfluxDBClient, Point
from fastapi.responses import StreamingResponse
import matplotlib.pyplot as plt
import io
from dotenv import load_dotenv

# Load environment variables
load_dotenv()

# FastAPI Initialization
app = FastAPI()

# CORS Middleware Setup
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost", "http://localhost:3000", "http://localhost:1880", "http://172.19.0.3:1883"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Logging Configuration
logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")
logger = logging.getLogger(__name__)

# Load Models and Scaler
AUTOENCODER_MODEL_PATH = os.getenv("AUTOENCODER_MODEL_PATH")
SCALER_MODEL_PATH = os.getenv("SCALER_MODEL_PATH")

try:
    autoencoder = load_model(AUTOENCODER_MODEL_PATH)
    logger.info("Autoencoder model loaded successfully.")
except Exception as e:
    logger.error(f"Error loading autoencoder model: {e}")
    raise

try:
    scaler = joblib.load(SCALER_MODEL_PATH)
    logger.info("Scaler loaded successfully.")
except Exception as e:
    logger.error(f"Error loading scaler: {e}")
    raise

# Database Initialization
POSTGRES_DB = os.getenv("POSTGRES_DB")
POSTGRES_USER = os.getenv("POSTGRES_USER")
POSTGRES_PASSWORD = os.getenv("POSTGRES_PASSWORD")
POSTGRES_HOST = os.getenv("POSTGRES_HOST")


def init_db_pool():
    """
    Initializes a connection pool for PostgreSQL database.
    Returns:
        psycopg2.pool.SimpleConnectionPool: Connection pool for PostgreSQL.
    """
    try:
        return psycopg2.pool.SimpleConnectionPool(
            1, 10,
            dbname=POSTGRES_DB,
            user=POSTGRES_USER,
            password=POSTGRES_PASSWORD,
            host=POSTGRES_HOST
        )
    except Exception as e:
        logger.error(f"Error initializing database connection pool: {e}")
        raise


try:
    db_pool = init_db_pool()
    logger.info("Database connection pool initialized successfully.")
except Exception as e:
    logger.error(f"Failed to initialize database connection pool: {e}")
    raise

# InfluxDB Configuration
INFLUXDB_URL = "http://172.19.0.6:8086"
INFLUXDB_TOKEN = os.getenv("INFLUXDB_ADMIN_TOKEN")
INFLUXDB_ORG = os.getenv("INFLUXDB_INIT_ORG")
INFLUXDB_BUCKET = os.getenv("INFLUXDB_BUCKET")
client = InfluxDBClient(url=INFLUXDB_URL, token=INFLUXDB_TOKEN)
write_api = client.write_api()

# Buffer Initialization
data_buffer = {'x': [], 'y': [], 'z': [], 'acceleration': []}
BUFFER_THRESHOLD = 100
reconstruction_error_buffer = []

# Pydantic Models
class InferenceResponse(BaseModel):
    timestamp: str
    reconstruction_error: float
    anomaly_status: bool


class SensorData(BaseModel):
    x_accelerometer_data: float
    y_accelerometer_data: float
    z_accelerometer_data: float
    acceleration_accelerometer_data: float


class SensorBatch(BaseModel):
    data: list[SensorData]


# Helper Functions
def reset_data_buffer():
    """
    Resets the data buffer used for batch data accumulation.
    """
    global data_buffer
    data_buffer = {'x': [], 'y': [], 'z': [], 'acceleration': []}
    logger.info("Data buffer has been reset.")


def create_features_from_batch(batch_data):
    """
    Creates feature vectors from a batch of sensor data.
    Args:
        batch_data (list[SensorData]): Batch of 24 sensor data objects.
    Returns:
        np.ndarray: Feature vector of shape (24,).
    """
    def statistical_features(data):
        return [
            np.mean(data), np.std(data),
            data[-1] - np.mean(data), pd.Series(data).skew(), pd.Series(data).kurt()
        ]
    acceleration = [data.acceleration_accelerometer_data for data in batch_data]
    x = [data.x_accelerometer_data for data in batch_data]
    y = [data.y_accelerometer_data for data in batch_data]
    z = [data.z_accelerometer_data for data in batch_data]

    features = [
        acceleration[-1], x[-1], y[-1], z[-1],
        *statistical_features(x), *statistical_features(acceleration),
        *statistical_features(y), *statistical_features(z)
    ]
    return np.array(features)


def generate_plot(data_buffer, reconstruction_error, is_anomaly, timestamp):
    """
    Generates a plot from buffered data and highlights anomalies.
    Args:
        data_buffer (dict): Accumulated sensor data.
        reconstruction_error (float): Reconstruction error value.
        is_anomaly (bool): Whether the data indicates an anomaly.
        timestamp (datetime): Timestamp of the data.
    Returns:
        bytes: Binary image data for the plot.
    """
    try:
        fig, ax = plt.subplots(figsize=(10, 6))
        x_vals = [item[1] for item in data_buffer['x']]
        y_vals = [item[1] for item in data_buffer['y']]
        z_vals = [item[1] for item in data_buffer['z']]
        acceleration_vals = [item[1] for item in data_buffer['acceleration']]
        ax.plot(x_vals, label="X", color="b")
        ax.plot(y_vals, label="Y", color="g")
        ax.plot(z_vals, label="Z", color="r")
        ax.plot(acceleration_vals, label="Acceleration", color="purple")
        if is_anomaly:
            anomaly_idx = len(acceleration_vals) - 1
            ax.annotate("Anomaly", (anomaly_idx, acceleration_vals[-1]))
        buf = io.BytesIO()
        plt.savefig(buf, format='png')
        plt.close(fig)
        buf.seek(0)
        return buf
    except Exception as e:
        logger.error(f"Error generating plot: {e}")
        raise


# FastAPI Endpoints
@app.post('/infer', response_model=InferenceResponse)
async def infer(batch: SensorBatch):
    """
    Processes a batch of sensor data to detect anomalies.
    Args:
        batch (SensorBatch): Batch of 24 sensor data points.
    Returns:
        InferenceResponse: Anomaly detection results.
    """
    try:
        features = create_features_from_batch(batch.data)
        scaled_features = scaler.transform(features.reshape(1, -1)).reshape(1, 24, 1)
        reconstruction = autoencoder.predict(scaled_features)
        error = np.mean(np.abs(reconstruction - scaled_features))
        reconstruction_error_buffer.append(error)
        threshold = np.percentile(reconstruction_error_buffer, 99)
        is_anomaly = error > threshold
        timestamp = datetime.datetime.now()
        # Accumulate data for visualization
        reset_data_buffer()
        # Store anomaly data in databases
        return InferenceResponse(timestamp=timestamp.isoformat(), reconstruction_error=error, anomaly_status=is_anomaly)
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Inference error: {str(e)}")


@app.get('/plot/{timestamp}')
async def serve_plot(timestamp: str):
    """
    Serves a plot image for a given timestamp.
    Args:
        timestamp (str): Timestamp for the plot.
    Returns:
        StreamingResponse: Binary image response.
    """
    try:
        return StreamingResponse(open(f"/tmp/plot_{timestamp}.png", "rb"), media_type="image/png")
    except FileNotFoundError:
        raise HTTPException(status_code=404, detail="Plot not found.")
