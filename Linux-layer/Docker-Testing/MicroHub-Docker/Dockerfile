FROM nodered/node-red
USER root
#USER root
RUN addgroup node-red dialout
USER node-red
RUN npm install node-red-node-serialport@0.15.0 node-red-dashboard
#RUN npm install node-red-dashboard

