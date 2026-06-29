FROM python:3.11-slim

WORKDIR /app
COPY udp-sender/sender.py .

# Если будут зависимости
# COPY requirements.txt .
# RUN pip install --no-cache-dir -r requirements.txt

ENTRYPOINT ["python", "sender.py"]