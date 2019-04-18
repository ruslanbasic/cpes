#!/usr/bin/python3

from http.server import BaseHTTPRequestHandler
import json
import os
import datetime
import wave
import contextlib


class StoreHandler(BaseHTTPRequestHandler):

  curdir = os.path.dirname(os.path.abspath(__file__))
  store_path = os.path.join(curdir, 'history/')

  def __init__(self, *args, **kwargs):
    os.makedirs(self.store_path, exist_ok=True)
    super().__init__(*args, **kwargs)

  def do_POST(self):
    if self.path.startswith('/speech-api/v2/recognize'):
      length = self.headers['content-length']
      data = self.rfile.read(int(length))
      filename = datetime.datetime.utcnow().strftime('%Y%m%dT%H%M%SZ.wav')
      filename = os.path.join(self.store_path, filename)
      with contextlib.closing(wave.open(filename, 'wb')) as wavfile:
        wavfile.setnchannels(1)
        wavfile.setsampwidth(2)
        wavfile.setframerate(16000)
        wavfile.writeframes(bytearray(data))

      self.send_response(200)
      self.end_headers()
      resp = json.dumps({
          "result":[]
      })
      self.wfile.write(bytes(resp, "utf-8"))
      self.wfile.write(bytes("\n", "utf-8"))
      resp = json.dumps({
          "result":[{"alternative":[{"transcript":"Включи свет","confidence":0.9129414},{"transcript":"Включить свет"}],"final":True}],"result_index":0
      }, ensure_ascii=False)
      self.wfile.write(bytes(resp, "utf-8"))


if __name__ == '__main__':
  from http.server import HTTPServer
  server = HTTPServer(('', 8888), StoreHandler)
  sa = server.socket.getsockname()
  print ('Starting server at http://{}:{}'.format(sa[0], sa[1]))
  server.serve_forever()

# curl -X POST --data-binary @brooklyn.flac 'http://0.0.0.0:8888/speech-api/v2/recognize'