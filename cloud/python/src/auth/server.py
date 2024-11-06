import jwt, datetime, os
from flask import Flask, request
from flask_mysqldb import MySQL
import bcrypt


server = Flask(__name__)
mysql = MySQL(server)

# config

server.config["MYSQL_HOST"] = os.environ.get("MYSQL_HOST")
server.config["MYSQL_USER"] = os.environ.get("MYSQL_USER")
server.config["MYSQL_PASSWORD"] = os.environ.get("MYSQL_PASSWORD")
server.config["MYSQL_DB"] = os.environ.get("MYSQL_DB")
server.config["MYSQL_PORT"] = int(os.environ.get("MYSQL_PORT"))


@server.route("/login", methods=["POST"])
def login():
    auth = request.authorization    
    if not auth:
        return "missing credentials", 401

    # check db for username and password
    cur = mysql.connection.cursor()
    res = cur.execute(
        "SELECT email, password, salt, role_id FROM users WHERE email=%s", (auth.username,)
    )
    if res > 0:
        user_row = cur.fetchone()

        email = user_row[0]
        print('user email : {}'.format(email))
        
        hashed_password_db = user_row[1].encode()
        print('user hash password : {}'.format(hashed_password_db))
        
        salt = user_row[2].encode()
        print('user salt : {}'.format(salt))

        role = user_row[3]

        tmp_password = auth.password.encode() + salt
        tmp_hashed_password = bcrypt.hashpw(tmp_password, salt)

        print(tmp_hashed_password)
        print(hashed_password_db)

        if tmp_hashed_password != hashed_password_db:
            return "invalid credentials", 401
        else:
            return createJWT(auth.username, "sarcasm", role)
    else:
        return "invalide credentials", 401



@server.route("/validate", methods=["POST"])
def validate():
    encoded_jwt = request.headers["Authorization"]
    print(encoded_jwt, flush=True)

    if not encoded_jwt:
        return "missing credentials", 401

    encoded_jwt = encoded_jwt.split(" ")[1]

    try:
        decoded = jwt.decode(
            encoded_jwt, "sarcasm", algorithms=["HS256"]
        )
        print(decoded, flush=True)
    except:
        return "not authorized", 403
    
    #check jwt 
    #check payload 

    return decoded, 200


def createJWT(username, secret, authz):
    return jwt.encode(
        {
            "username": username,
            "exp": datetime.datetime.now(tz=datetime.timezone.utc) # time of validity of the token to access services
            + datetime.timedelta(days=1),
            "iat": datetime.datetime.utcnow(),                     # issuing tile of the token 
            "role": authz,                                        # define the services the client will have access
        },
        secret,
        algorithm="HS256",
    )

# check if token valid
def token_validation(token):
    #check experitation date 
    return True 
# check is paylod 
def payload_validation(payload):
    return True

if __name__ == "__main__":
    server.run(host="0.0.0.0", port=5000)