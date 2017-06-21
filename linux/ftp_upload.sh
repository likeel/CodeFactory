#!/bin/bash

CLIENT_PATH="/home/torrent/downloads/barosvr_video/*";
CLIENT_BACKUP_PATH="/home/torrent/downloads/barosvr_video_bk";


SERVER_ADDRESS="test@111.222.333.444";
SERVER_PORT="22";
SERVER_PATH="/data/user/lik/video";


scp -P $SERVER_PORT -r $CLIENT_PATH $SERVER_ADDRESS:$SERVER_PATH

mv $CLIENT_PATH $CLIENT_BACKUP_PATH
