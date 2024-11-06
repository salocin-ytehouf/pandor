from pyfiglet import figlet_format
from termcolor import colored
import numpy as np
import logging
import os
import sys
import cv2
import time
import traceback
import keyboard                            
import getpass
import mysql.connector
import bcrypt
import base64
import cv2
import json
import subprocess
import face_recognition
import pymongo
from pymongo import MongoClient
from bson.objectid import ObjectId
from gridfs import GridFS
import gridfs
from PIL import Image
import io
import numpy as np
import face_recognition
import os
import configparser

def parse_ini_file(file_path='./config.ini'):
    config = configparser.ConfigParser()
    config.read(file_path)

    for section in config.sections():
        for key, value in config.items(section):
            env_var_name = f'PANDOR_{section.upper()}_{key.upper()}'
            os.environ[env_var_name] = value

def print_env_variables():
    pandor_vars = {key: value for key, value in os.environ.items() if key.startswith('PANDOR')}
    for key, value in pandor_vars.items():
        print(f'{key}: {value}')

global_jwt = ''

def log(string, color, font="slant", figlet=False):
    if colored:
        if not figlet:
            print(colored(string, color))
        else:
            print(colored(figlet_format(
                string, font=font), color))
    else:
        print(string)

#########################################################
###################### USER MENU ########################

def print_menu(menu_options, db_params):

    """Print menu to console"""
    os.system("clear")
    while True:
        log("welcome to PANDOR", color="blue", figlet=True)
        print("Choose option you want to use : ")
        print()

        for (key, value) in sorted(menu_options.items()):
            print("%s: %s" % (key, value[1]))
        print()

        choice = input("Option>> ")
        choice.lower()
        print()


        if choice in menu_options:
            if isinstance(menu_options[choice][0], str):
                if menu_options[choice][0] == 'register_new_user':
                    os.system("clear")
                    create_new_user(db_params)
                elif menu_options[choice][0] == 'get_list_user':
                    os.system("clear")
                    list_all_users(db_params)
                elif menu_options[choice][0] == 'remove_user':
                    os.system("clear")
                    delete_user(db_params)
                elif menu_options[choice][0] == 'create_jwt':
                    os.system("clear")
                    create_jwt_user()
                elif menu_options[choice][0] == 'print_global_jwt':
                    os.system("clear")
                    print_global_jwt()   
            else:
                return
        else:
            print("Option not implemented. Try again!")
            

        input("Press enter to continue")
        os.system("clear")

def template_user_menu(db_params):
    """"""
    menu_options = {
        '1': ['register_new_user', "Register new pandore user"],
        '3': ['remove_user', "Remove specific pandor user"],
        '4': ['get_list_user', "Get all pandor user list "],
        '5': ['create_jwt', "create new valid jwt token for specific user"],
        '6': ['print_global_jwt', "print last jwt token"],
        'q': [None, 'Return to main menu']
    }
    print_menu(menu_options, db_params)

def list_all_users(db_params):

    db_connection = mysql.connector.connect(**db_params)
    
    cursor = db_connection.cursor()
    select_query = "SELECT * FROM users"
    cursor.execute(select_query)
    users = cursor.fetchall()
    
    cursor.close()
    db_connection.close()
    
    for user in users:
        print(user)
    
    return

def create_new_user(db_params):
    username = input("Enter your username: ")
    email = input("Enter your email address: ")
    password = input("Enter your password: ")
    role_id = input("Enter your role ID: ")
    db_connection = mysql.connector.connect(**db_params)
    
    salt = bcrypt.gensalt()
    combined_password = password.encode() + salt
    hashed_password = bcrypt.hashpw(combined_password, salt)
    
    cursor = db_connection.cursor()
    insert_query = "INSERT INTO users (username, email, password, salt, role_id) VALUES (%s, %s, %s, %s, %s)"
    user_data = (username, email, hashed_password, salt, role_id)
    cursor.execute(insert_query, user_data)
    
    db_connection.commit()
    cursor.close()
    db_connection.close()

def delete_user(db_params):
    user = input("Enter your username: ")
    try:
        db_connection = mysql.connector.connect(**db_params)
        cursor = db_connection.cursor()

        # Check if the username exists in the database
        cursor.execute("SELECT username FROM users WHERE username = %s", (user,))
        existing_user = cursor.fetchone()

        if existing_user:
            # If the username exists, delete the user
            cursor.execute("DELETE FROM users WHERE username = %s", (user,))
            db_connection.commit()
            print(f"User '{user}' deleted successfully!")
        else:
            print(f"User '{user}' not found in the database.")
            input()

    except mysql.connector.Error as err:
        print(f"Error: {err}")

    finally:
        cursor.close()
        db_connection.close()

def create_jwt_user():
    username = input("Enter your username: ")
    password = input("Enter your password: ")

    url = "http://pandor.com/login"
    credentials = f"{username}:{password}"

    try:
        # Construct the curl command
        curl_command = [
            "curl",
            "-X", "POST",
            url,
            "-u", credentials
        ]

        # Execute the curl command
        result = subprocess.run(curl_command, capture_output=True, text=True, check=True)

        # Return the command output (response)
        print(result.stdout)
        global global_jwt
        global_jwt = result.stdout
    
    except subprocess.CalledProcessError as e:
        return f"Error: {e}\n{e.stderr}"
    return

def init_jwt_user():
    username = os.getenv('PANDOR_API_USERNAME')
    password = os.getenv('PANDOR_API_PASSWORD')

    url = "http://pandor.com/login"
    credentials = f"{username}:{password}"

    try:
        # Construct the curl command
        curl_command = [
            "curl",
            "-X", "POST",
            url,
            "-u", credentials
        ]

        # Execute the curl command
        result = subprocess.run(curl_command, capture_output=True, text=True, check=True)

        # Return the command output (response)
        print(result.stdout)
        global global_jwt
        global_jwt = result.stdout
    
    except subprocess.CalledProcessError as e:
        return f"Error: {e}\n{e.stderr}"
    return

def print_global_jwt():
    global global_jwt
    print(global_jwt)


#########################################################
################ FACE AND PLATE MENU ####################

def print_face_plate_menu(menu_options, db_params):

    """Print menu to console"""
    os.system("clear")
    while True:
        log("welcome to PANDOR", color="blue", figlet=True)
        print("Choose option you want to use : ")
        print()

        for (key, value) in sorted(menu_options.items()):
            print("%s: %s" % (key, value[1]))
        print()

        choice = input("Option>> ")
        choice.lower()
        print()

        if choice in menu_options:
            if isinstance(menu_options[choice][0], str):
                if menu_options[choice][0] == 'add_authorized_plate':
                    os.system("clear")
                    add_authorized_plate(db_params)
                elif menu_options[choice][0] == 'remove_authorized_plate':
                    os.system("clear")
                    remove_authorized_plate(db_params)      
                elif menu_options[choice][0] == 'get_list_plate':
                    os.system("clear")
                    get_list_plate(db_params)
                elif menu_options[choice][0] == 'add_authorized_face':
                    os.system("clear")
                    add_authorized_face(db_params)
                elif menu_options[choice][0] == 'remove_authorized_face':
                    os.system("clear")
                    remove_authorized_face(db_params)      
                elif menu_options[choice][0] == 'get_list_face':
                    os.system("clear")
                    get_list_face(db_params)  
            else:
                return
        else:
            print("Option not implemented. Try again!")
            

        input("Press enter to continue")
        os.system("clear")

def template_face_plate_menu(db_params):
    menu_options = {
        '1': ['add_authorized_face', "add new authorized face"],
        '2': ['remove_authorized_face', "remove authorized face"],
        '3': ['get_list_face', "get list uthorized face"],
        '4': ['add_authorized_plate', "add new authorized plate"],
        '5': ['remove_authorized_plate', "remove authorized plate"],
        '6': ['get_list_plate', "get list authorized plate"],
        'q': [None, 'Return to main menu']
    }
    print_face_plate_menu(menu_options, db_params)

def add_authorized_face(db_params):
    default_image=input('use default images ? y or n: ')
    if default_image == 'y':
        img_path = './data/add_face.jpeg'
    else:
        img_path = input('enter the path of new face to add: ')
    adm_id = input('enter adm id: ')
    worker_id = input('enter worker id: ')
    user_id   = input('enter user id: ')
    camera_id = input('enter camera id: ')

    image = face_recognition.load_image_file(img_path)
    encoding = face_recognition.face_encodings(image, model = "large")[0]
    print(encoding)
    data = {
        "adm_id" : adm_id,
        "encoding": encoding.tolist(),
        "user_id" : worker_id,
        "camera_id": camera_id,
        "worker_id": user_id,
        "ts": str(time.time())  
    }

    # Connect to MongoDB (adjust the MongoDB URI as needed)
    client = pymongo.MongoClient("mongodb://localhost:27017/")

    # Specify the database "auth_face"
    db = client["auth_face"]

    # Specify the collection where you want to insert data (e.g., "users")
    collection = db["users"]

    try:
        # Insert the data into the collection
        result = collection.insert_one(data)
        print("Inserted document ID:", result.inserted_id)
    except Exception as e:
        print("Error inserting data:", e)
    finally:
        # Close the MongoDB client connection
        client.close()
    return

def add_authorized_plate(db_params):
    worker_id = input('enter worker id: ')
    adm_id = input('enter adm id: ')
    user_id   = input('enter user id: ')
    plate_id  = input('enter plate id: ')
    camera_id = input('enter camera id: ')

    data = {
        "adm_id" : adm_id,
        "plate_id": plate_id,
        "user_id" : worker_id,
        "camera_id": camera_id,
        "worker_id": user_id,
        "ts": str(time.time())  
    }
    #upload encoding mongodb
    # Connect to MongoDB (adjust the MongoDB URI as needed)
    client = pymongo.MongoClient("mongodb://localhost:27017/")

    # Specify the database "auth_face"
    db = client["auth_plate"]
    # Specify the collection where you want to insert data (e.g., "users")
    collection = db["users"]

    try:
        # Insert the data into the collection
        result = collection.insert_one(data)
        print("Inserted document ID:", result.inserted_id)
    except Exception as e:
        print("Error inserting data:", e)
    finally:
        # Close the MongoDB client connection
        client.close()
    return

def get_list_face(db_params):
    client = pymongo.MongoClient("mongodb://localhost:27017/")
    db = client["auth_face"]
    # Specify the collection where you want to insert data (e.g., "users")
    collection = db["users"]
    res = collection.find()
    for documents in res:
        print(documents)

    client.close()
    return

def get_list_plate(db_params):
    client = pymongo.MongoClient("mongodb://localhost:27017/")
    # Specify the database "auth_face"
    db = client["auth_plate"]
    # Specify the collection where you want to insert data (e.g., "users")
    collection = db["users"]
    res = collection.find()
    for documents in res:
        print(documents)

    client.close()
    return

def remove_authorized_face(db_params):
    face_id = input("Enter the face_id you want to remove: ")
    try:
        db_connection = mysql.connector.connect(**db_params)
        cursor = db_connection.cursor()

        # Check if the plate descriptor exists in the "authorized_plates" table
        cursor.execute("SELECT face_id FROM authorized_faces WHERE face_id = %s", (face_id,))
        face = cursor.fetchone()

        if face:
            # If the plate descriptor exists, delete the entry
            cursor.execute("DELETE FROM authorized_faces WHERE face_id = %s", (face_id,))
            db_connection.commit()
            print(f"face '{face_id}' removed successfully!")
        else:
            print(f"face '{face_id}' not found in the database.")

    except mysql.connector.Error as err:
        print(f"Error: {err}")

    finally:
        cursor.close()
        db_connection.close()

def remove_authorized_plate(db_params):
    plate_descriptor = input("Enter the plate number you want to remove: ")
    try:
        db_connection = mysql.connector.connect(**db_params)
        cursor = db_connection.cursor()

        # Check if the plate descriptor exists in the "authorized_plates" table
        cursor.execute("SELECT plate_id FROM authorized_plates WHERE plate_descriptor = %s", (plate_descriptor,))
        plate = cursor.fetchone()

        if plate:
            # If the plate descriptor exists, delete the entry
            cursor.execute("DELETE FROM authorized_plates WHERE plate_descriptor = %s", (plate_descriptor,))
            db_connection.commit()
            print(f"Plate '{plate_descriptor}' removed successfully!")
        else:
            print(f"Plate '{plate_descriptor}' not found in the database.")

    except mysql.connector.Error as err:
        print(f"Error: {err}")

    finally:
        cursor.close()
        db_connection.close()

#########################################################
####################### DB MENU #########################

def print_db_menu(menu_options, db_params):

    """Print menu to console"""
    os.system("clear")
    while True:
        log("welcome to PANDOR", color="blue", figlet=True)
        print("Choose option you want to use : ")
        print()

        for (key, value) in sorted(menu_options.items()):
            print("%s: %s" % (key, value[1]))
        print()

        choice = input("Option>> ")
        choice.lower()
        print()

        if choice in menu_options:
            if isinstance(menu_options[choice][0], str):
                if menu_options[choice][0] == 'mysql_describe_tables_in_db':
                    os.system("clear")
                    mysql_describe_tables(db_params)
                elif menu_options[choice][0] == 'mysql_show_table':
                    os.system("clear")
                    mysql_show_table(db_params)
                elif menu_options[choice][0] == 'mongo_show_table':
                    os.system("clear")
                    mongo_show_table()     
                elif menu_options[choice][0] == 'mongo_images_clean_table':
                    os.system("clear")
                    mongo_images_clean_table()   
                elif menu_options[choice][0] == 'mongo_show_table_res':
                    os.system("clear")
                    mongo_show_table_res()     
                elif menu_options[choice][0] == 'mongo_res_clean_table':
                    os.system("clear")
                    mongo_res_clean_table()
                elif menu_options[choice][0] == 'mongo_show_table_plate':
                    os.system("clear")
                    mongo_show_table_plate()     
                elif menu_options[choice][0] == 'mongo_plate_clean_table':
                    os.system("clear")
                    mongo_plate_clean_table()  
                elif menu_options[choice][0] == 'mongo_show_table_face':
                    os.system("clear")
                    mongo_show_table_face()     
                elif menu_options[choice][0] == 'mongo_face_clean_table':
                    os.system("clear")
                    mongo_face_clean_table()  
                    
                elif menu_options[choice][0] == 'mongo_show_table_auth_face':
                    os.system("clear")
                    mongo_show_table_auth_face()     
                elif menu_options[choice][0] == 'mongo_auth_face_clean_table':
                    os.system("clear")
                    mongo_auth_face_clean_table() 
                elif menu_options[choice][0] == 'mongo_show_table_auth_plate':
                    os.system("clear")
                    mongo_show_table_auth_plate()     
                elif menu_options[choice][0] == 'mongo_auth_plate_clean_table':
                    os.system("clear")
                    mongo_auth_plate_clean_table()  
            else:
                return
        else:
            print("Option not implemented. Try again!")
            

        input("Press enter to continue")
        os.system("clear")

def template_db_menu(db_params):
    menu_options = {
        '1': ['mysql_describe_tables_in_db', "mysql: describe all tables in db"],
        '2': ['mysql_show_table', "mysql: show tables elements"],
        '3': ['mongo_show_table', "mongo image show table"],
        '4': ['mongo_images_clean_table', "mongo image clean table"],
        '5': ['mongo_show_table_res', "mongo result show result table"],
        '6': ['mongo_res_clean_table', "mongo result clean result table"],
        '7': ['mongo_show_table_plate', "mongo plate show table"],
        '8': ['mongo_plate_clean_table', "mongo plate clean table"],
        '9': ['mongo_show_table_face', "mongo face show table"],
        'a': ['mongo_face_clean_table', "mongo face clean table"],
        'b': ['mongo_show_table_auth_face', "mongo auth_face show table"],
        'c': ['mongo_auth_face_clean_table', "mongo auth_face clean table"],
        'd': ['mongo_show_table_auth_plate', "mongo auth_plate show table"],
        'e': ['mongo_auth_plate_clean_table', "mongo auth_plate clean table"],
        'q': [None, 'Return to main menu']
    }
    print_db_menu(menu_options, db_params)

def mysql_describe_tables(db_params):
    try:
        db_connection = mysql.connector.connect(**db_params)
        cursor = db_connection.cursor()

        # Get the list of tables in the database
        cursor.execute("SHOW TABLES")
        tables = cursor.fetchall()

        # Initialize max_width to 0
        max_width = 0

        # Display tables and their descriptions
        for table in tables:
            table_name = table[0]
            print(f"Table: {table_name}")
            
            # Describe the table
            cursor.execute(f"DESCRIBE {table_name}")
            table_description = cursor.fetchall()

            # Determine the maximum width for the column names (desc[0])
            max_width = max(max_width, max(len(desc[0]) for desc in table_description))

            for desc in table_description:
                # Align desc[0] and desc[1] with the maximum width
                print(f"  {desc[0]:<{max_width}} -      {desc[1]}")
            print("")
    except mysql.connector.Error as err:
        print(f"Error: {err}")

    finally:
        cursor.close()
        db_connection.close()
    
def mysql_show_table(db_params):
    table_name = input("Enter table_name: ")
    try:
        db_connection = mysql.connector.connect(**db_params)
        cursor = db_connection.cursor()

        # Select all rows from the specified table
        cursor.execute(f"SELECT * FROM {table_name}")
        rows = cursor.fetchall()

        # Print all rows
        for row in rows:
            print(row)

    except mysql.connector.Error as err:
        print(f"Error: {err}")

    finally:
        cursor.close()
        db_connection.close()

def mongo_show_table():
    database_name='images'
    database_url='mongodb://localhost:27017'

    try:
        client = MongoClient(database_url)

        # Access the specified database
        db = client[database_name]

        # Access the GridFS collection
        fs = GridFS(db)

        # Iterate through all files in the GridFS collection
        for file_info in fs.find():
            print(f"File ID: {file_info._id}, Upload Date: {file_info.upload_date}, File Size: {file_info.length} bytes")


    except Exception as e:
        print(f"Error: {e}")

    finally:
        # Close the MongoDB connection
        client.close()

def mongo_images_clean_table():
    database_name='images'
    database_url='mongodb://localhost:27017'
    
    try:
        client = MongoClient(database_url)

        # Access the specified database
        db = client[database_name]
        print(db.list_collection_names())

        # Access the GridFS collection
        fs = GridFS(db)

        # Find and delete all files in the GridFS collection
        for file_info in fs.find():
            print(file_info)
            fs.delete(file_info._id)

        print(f"All files and chunks in the '{collection_name}' collection have been deleted.")

    except Exception as e:
        print(f"Error: {e}")

    finally:
        # Close the MongoDB connection
        client.close()

def mongo_show_table_res():

    client = pymongo.MongoClient("mongodb://localhost:27017/")
    db = client['detector']
    collection = db['res']
    res = collection.find()
    for documents in res:
        print(documents)

    client.close()
    return

def mongo_res_clean_table():

    client = pymongo.MongoClient("mongodb://localhost:27017/")
    db = client['detector']
    collection = db['res']
    res = collection.find()
    for document in res:
        collection.delete_one({"_id": document["_id"]})
        print('deleting document with id {}'.format(document["_id"]))

    client.close()
    return

def mongo_show_table_plate():
    client = pymongo.MongoClient("mongodb://localhost:27017/")
    db = client['plates']
    collection = db['plates']
    res = collection.find()
    for documents in res:
        print(documents)
    client.close()
    return

def mongo_plate_clean_table():
    client = pymongo.MongoClient("mongodb://localhost:27017/")
    db = client['plates']
    collection = db['plates']
    res = collection.find()
    for document in res:
        collection.delete_one({"_id": document["_id"]})
        print('deleting document with id {}'.format(document["_id"]))

    client.close()
    return

def mongo_show_table_face():
    client = pymongo.MongoClient("mongodb://localhost:27017/")
    db = client['faces']
    collection = db['res']
    res = collection.find()
    for documents in res:
        print(documents)

    client.close()
    return

def mongo_show_table_auth_face():
    client = pymongo.MongoClient("mongodb://localhost:27017/")
    db = client["auth_face"]
    # Specify the collection where you want to insert data (e.g., "users")
    collection = db["users"]
    res = collection.find()
    for documents in res:
        print(documents)

    client.close()
    return

def mongo_auth_face_clean_table():
    client = pymongo.MongoClient("mongodb://localhost:27017/")
    db = client["auth_face"]
    # Specify the collection where you want to insert data (e.g., "users")
    collection = db["users"]
    res = collection.find()
    for document in res:
        collection.delete_one({"_id": document["_id"]})
        print('deleting document with id {}'.format(document["_id"]))

    client.close()
    return

def mongo_show_table_auth_plate():
    client = pymongo.MongoClient("mongodb://localhost:27017/")
    # Specify the database "auth_face"
    db = client["auth_plate"]
    # Specify the collection where you want to insert data (e.g., "users")
    collection = db["users"]
    res = collection.find()
    for documents in res:
        print(documents)

    client.close()
    return

def mongo_auth_plate_clean_table():
    client = pymongo.MongoClient("mongodb://localhost:27017/")
    # Specify the database "auth_face"
    db = client["auth_plate"]
    # Specify the collection where you want to insert data (e.g., "users")
    collection = db["users"]
    res = collection.find()
    for document in res:
        collection.delete_one({"_id": document["_id"]})
        print('deleting document with id {}'.format(document["_id"]))

    client.close()
    return

#########################################################
################## TEST IMAGE WORKER ####################

def print_image_menu(menu_options):

    """Print menu to console"""
    os.system("clear")
    while True:
        log("welcome to PANDOR", color="blue", figlet=True)
        print("Choose option you want to use : ")
        print()

        for (key, value) in sorted(menu_options.items()):
            print("%s: %s" % (key, value[1]))
        print()

        choice = input("Option>> ")
        choice.lower()
        print()

        if choice in menu_options:

            
            if isinstance(menu_options[choice][0], str):
        
                if menu_options[choice][0] == 'cloud_image_processing':
                    os.system("clear")
                    cloud_image_processing()
          
                elif menu_options[choice][0] == 'test_detector_res':
                    os.system("clear")
                    test_detector_res() 
            else:
                return
        else:
            print("Option not implemented. Try again!")
            

        input("Press enter to continue")
        os.system("clear")

def template_image_menu():
    menu_options = {
        '1': ['cloud_image_processing', "test cloud imag processing"],
        '2': ['test_detector_res', "test detector result"],
        'q': [None, 'Return to main menu']
    }
    print_image_menu(menu_options)

def cloud_image_processing():
    print("#################################")

    global global_jwt
    global_jwt = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VybmFtZSI6Im5pY29AZ21haWwuY29tIiwiZXhwIjoxNjk4NjY0NjEyLCJpYXQiOjE2OTg1NzgyMTIsInJvbGUiOjF9.D6Zt3TgIKwy_-u0w8YI9e8-SFtjNsma0f3JTX-at8cE"
    url_to_post = "http://pandor.com/upload"
    # Capture images from camera
    # Load images using OpenCV
    default_image=input('use default images ? y or n: ')
    if default_image == 'y':
        image1 = cv2.imread('./data/1.jpeg')
        image2 = cv2.imread('./data/2.jpeg')
    else:
        path_1 = input('enter path to image 1 :')
        path_2 = input('enter path to image 2 :')
        image1 = cv2.imread(path_1)
        image2 = cv2.imread(path_2)
    print(type(image1))

    # Convert images to base64
    
    _, image1_buffer = cv2.imencode('.jpeg', image1)
    _, image2_buffer = cv2.imencode('.jpeg', image2)
    image1_base64 = base64.b64encode(image1_buffer).decode('utf-8')
    image2_base64 = base64.b64encode(image2_buffer).decode('utf-8')
    

    # Get current timestamp
    current_timestamp = int(time.time())

    # Prepare metadata for each image
    image1_metadata = {
        "worker_id": 'aaaaaa',
        "timestamp": current_timestamp,
        "camera_id": "camera1",
        "is_door": "True"
    }
    image2_metadata = {
        "worker_id": 'aaaaaa',
        "timestamp": current_timestamp,
        "camera_id": "camera2",
        "is_door": "False"
    }

    # Prepare JSON payload
    payload = {
        "image1": {
            "data": image1_base64,
            "metadata": image1_metadata
            },
        "image2": {
            "data": image2_base64,
            "metadata": image2_metadata
            }
    }
    
    payload_json = json.dumps(payload)
    #print(json.dumps(payload_json, indent=4))
    with open("payload.json", "w") as f:
        f.write(payload_json)
    # Use curl to send the request with the payload file
    
    curl_command = f'curl -X POST -H "Authorization: Bearer {global_jwt}" -H "Content-Type: application/json" -d @payload.json {url_to_post}'
    print(curl_command)
    subprocess.run(curl_command, shell=True)
    '''
    # Use curl to send the request
    curl_command = f'curl -X POST -H "Authorization: Bearer {global_jwt}" -H "Content-Type: application/json" -d \'{payload_json}\' {url_to_post}'
    
    # Run the curl command using subprocess
    subprocess.run(curl_command, shell=True)
    '''

def load_img_from_mongo(id_img):
    client = pymongo.MongoClient("mongodb://localhost:27017/")
    db_image = client.images
    fs_images = gridfs.GridFS(db_image)
    img_gridout = fs_images.get(ObjectId(id_img))
    img_binary_data = img_gridout.read()
    nparr = np.frombuffer(img_binary_data, np.uint8)
    image = cv2.imdecode(nparr, cv2.IMREAD_COLOR)  
    img = np.asarray(image)
    client.close()
    return img

def load_res_from_mongo(id_img):
    client = pymongo.MongoClient("mongodb://localhost:27017/")
    db = client['detector']
    collection = db['res']
    filter_criteria = {"image_fid": id_img}
    res = collection.find(filter_criteria)
    documents = list(res)
    client.close()

    return documents

def draw_bboxes_on_image(img, detections):
    # Make a copy of the image to avoid modifying the original
    image_with_boxes = img.copy()

    # Define colors for drawing boxes (BGR format)
    box_color = (0, 255, 0)  # Green color
    text_color = (0, 0, 255)  # Red color for text

    for key, detection in detections.items():
        if key.isdigit():
            bbox = detection.get("bbox")
            label = detection.get("label")
            confidence = detection.get("confidence")

            # Extract coordinates of the bounding box
            x_min, y_min, x_max, y_max = [int(coord) for coord in bbox]

            # Draw the bounding box rectangle
            cv2.rectangle(image_with_boxes, (x_min, y_min), (x_max, y_max), box_color, 2)

            # Create a label string (e.g., "car: 0.90")
            label_text = f"{label}: {confidence:.2f}"

            # Define the position for the label text
            label_position = (x_min, y_min - 10)  # Above the box

            # Draw the label text on the image
            cv2.putText(image_with_boxes, label_text, label_position, cv2.FONT_HERSHEY_SIMPLEX, 0.5, text_color, 2)

    return image_with_boxes

def test_detector_res():
    id_img=input('enter img id you want to test :')
    image_data = load_img_from_mongo(id_img)
    result = load_res_from_mongo(id_img)
    for res in result:
        tmp = draw_bboxes_on_image(image_data, res)
        cv2.imshow('test', tmp)
        cv2.waitKey(0)
        cv2.destroyAllWindows()

#########################################################
###################### MAIN MENU ########################

def main():
    parse_ini_file()
    print_env_variables()
    
    init_jwt_user()

    db_params_user_sql = {
        "host": os.getenv('PANDOR_DB_MYSQL_HOST'),
        "user": os.getenv('PANDOR_DB_MYSQL_USER'),
        "password": os.getenv('PANDOR_DB_MYSQL_PASSWORD'),
        "database": os.getenv('PANDOR_DB_MYSQL_DATABASE')
    }

    menu_options = {
        'a': ['user_menu', "User sub menu"],
        'b': ['db_sub_menu', "DB sub menu"],
        'c': ['face_plate_sub_menu', "faces and plates sub menu"],
        'd': ['image_test_menu', "image test sub menu"],
        'q': [None, 'Exit']
    }

    while True:
        os.system("clear")
        log("welcome to PANDOR", color="blue", figlet=True)
        print("Choose option you want to use : ")
        print()
        for (key, value) in sorted(menu_options.items()):
            print("%s: %s" % (key, value[1]))
        print()

        choice = input("Option>> ")
        choice.lower()
        print()

        if choice in menu_options:
            if isinstance(menu_options[choice][0], str):
                if menu_options[choice][0] == "user_menu":
                    template_user_menu(db_params_user_sql)
                    continue
                elif menu_options[choice][0] == "db_sub_menu":
                    template_db_menu(db_params_user_sql)
                    continue
                elif menu_options[choice][0] == "face_plate_sub_menu":
                    template_face_plate_menu(db_params_user_sql)
                    continue
                elif menu_options[choice][0] == "image_test_menu":
                    template_image_menu()
                    continue
            else:
                return
        else:
            print("Option not implemented. Try again!")

        input("Press enter to continue")

if __name__ == "__main__":
    main()