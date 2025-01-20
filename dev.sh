#!/bin/bash

case "$1" in
  "start")
    docker-compose up --build
    ;;
  "start-dev")
    docker-compose -f docker-compose.dev.yml up --build
    ;;
  "stop")
    docker-compose down
    ;;
  "stop-dev")
    docker-compose -f docker-compose.dev.yml down
    ;;
  "rebuild")
    docker-compose down
    docker-compose build --no-cache
    docker-compose up
    ;;
  "clean")
    docker-compose down -v
    ;;
  "logs")
    docker-compose logs -f
    ;;
  "shell")
    docker-compose exec trading-server /bin/bash
    ;;
  *)
    echo "Usage: $0 {start|start-dev|stop|stop-dev|rebuild|clean|logs|shell}"
    exit 1
    ;;
esac