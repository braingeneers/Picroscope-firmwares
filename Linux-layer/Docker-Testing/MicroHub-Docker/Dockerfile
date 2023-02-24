FROM nodered/node-red
RUN npm install node-red-node-serialport node-red-dashboard 
USER root
RUN addgroup node-red dialout
USER node-red
