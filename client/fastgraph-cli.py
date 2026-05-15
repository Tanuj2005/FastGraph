import redis
import sys

r = redis.Redis(host='localhost', port=6379)
print(r.ping())
