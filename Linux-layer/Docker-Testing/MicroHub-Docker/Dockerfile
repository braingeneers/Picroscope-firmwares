FROM nodered/node-red

USER root
RUN apk add --no-cache aws-cli
RUN addgroup node-red dialout

USER node-red
RUN npm install node-red-node-serialport@0.15.0 node-red-dashboard

