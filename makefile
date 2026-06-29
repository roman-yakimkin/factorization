HOST ?= server

.PHONY: build up down logs server sender clean

.DEFAULT_GOAL := help

# Сборка образов
build:
	docker-compose build

# Запуск только сервера
server:
	docker-compose up server

# Запуск отправителя (в другом терминале)
sender:
	HOST=$(HOST) docker-compose run --rm sender --mode mixed

# Запуск всего (сервер + отправитель)
up:
	docker-compose up

# Логи в реальном времени
logs:
	docker-compose logs -f

# Остановка
down:
	docker-compose down

# Стресс-тест
simple:
	HOST=$(HOST) docker-compose run --rm sender --mode simple

# Стресс-тест
stress:
	HOST=$(HOST) docker-compose run --rm sender --mode stress --duration 30

# Burst тест
burst:
	HOST=$(HOST) docker-compose run --rm sender --mode burst --count 100

# Очистка
clean:
	docker-compose down -v --rmi all

# Справка
help: ## Показать эту справку
	@echo ""
	@echo "🔢 Factorization Demo - Доступные команды:"
	@echo ""
	@echo "  make <command>"
	@echo ""
	@echo "Команды:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@echo "Примеры использования:"
	@echo "  1. make build          # Собрать образы (один раз)"
	@echo "  2. make server         # Запустить сервер"
	@echo "  3. make sender         # В другом терминале запустить отправитель (будут переданы несколько разных чисел)"
	@echo "  4. make simple         # В другом терминале запустить отправитель (будут переданы числа от 1 до 1000)"
	@echo "  5. make stress         # В другом терминале запустить отправитель (будут переданы случайные числа в течение 30 секунд с максимальной скоростью отправки)"
	@echo "  6. make logs           # Смотреть логи"
	@echo "  7. make down           # Остановить всё"
	@echo ""