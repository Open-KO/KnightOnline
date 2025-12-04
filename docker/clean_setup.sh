# Linux script
cd ..

# Removes any existing sqlserver/kodb-util images/volumes, 
docker compose down --rmi all -v

# then creates/starts new versions of them.  
docker compose up --build sqlserver -d
docker compose up --build kodb-util -d

# Before the script completes, the script to create an up-to-date 
# OpenKO database will be invoked.
docker exec -it knightonline-kodb-util-1 /usr/local/bin/cleanImport.sh
