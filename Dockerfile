FROM gcc:latest

# Installation des dépendances nécessaires (OpenSSL)
RUN apt-get update && apt-get install -y libssl-dev nlohmann-json3-dev

WORKDIR /app

# Copier les fichiers sources et de configuration dans le conteneur
ADD app/lib .
ADD app/handlers .
ADD app/src .

COPY app/config.json .

RUN mkdir -p shared/datas

# Compilation du code source avec les options nécessaires
RUN g++ -std=c++17 -lcrypto -lssl -Wall -Werror -O3 main.cpp RoktService.cpp RoktDataset.cpp RoktData.cpp LogService.cpp EncryptService.cpp Config.cpp -o rokt_socket

# Exposer le port sur lequel le serveur socket écoute
EXPOSE 8080

# Lancer l'exécutable au démarrage du conteneur
CMD ["./rokt_socket"]