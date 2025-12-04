# Linux script
cd ..

# Performs a clean import of the OpenKO database.  This will 
# remove any existing OpenKO database instance matching the 
# DB_NAME from the .env file (default: KN_Online), then 
# create a replacement database using the latest data 
# from the OpenKO-db project.
docker exec knightonline-kodb-util-1 /usr/local/bin/cleanImport.sh
