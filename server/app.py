from flask import Flask, request, jsonify, Response, redirect, url_for
from waitress import serve
import logging
import json
import pymysql
import bcrypt
import dotenv
import os

dotenv.load_dotenv()

def get_db_connection():
    return pymysql.connect(
        host=os.getenv('HOST'),
        user=os.getenv('USER'),
        password=os.getenv('PASSWORD'),
        db=os.getenv('DATABASE'),
        cursorclass=pymysql.cursors.DictCursor
    )

app = Flask(__name__)

logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
app.logger.info('Logging setup complete')


@app.route('/', methods=['GET'])
def index():
    return Response(json.dumps({'message': 'Welcome to the Music API!'}), status=200, mimetype='application/json')


@app.route('/add_like', methods=['POST'])
def add_like():
    data = request.get_json()
    username = data.get('username')
    song_url = data.get('song_url')

    if not all([username, song_url]):
        return jsonify({'error': 'Missing data'}), 400

    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            cursor.execute("SELECT id FROM account WHERE username = %s", (username,))
            account = cursor.fetchone()
            if account is None:
                return jsonify({'error': 'Account not found'}), 404

            cursor.execute("SELECT id FROM song WHERE url = %s", (song_url,))
            song = cursor.fetchone()
            if song is None:
                return jsonify({'error': 'Song not found'}), 404

            cursor.execute("INSERT IGNORE INTO account_song (account_id, song_id) VALUES (%s, %s)",
                           (account['id'], song['id']))
            conn.commit()

            return jsonify({'message': 'Like added successfully'}), 200
    except Exception as e:
        app.logger.error(f"Error: {e}")
        return jsonify({'error': 'Internal Server Error'}), 500
    finally:
        conn.close()
        

@app.route('/change_password', methods=['POST'])
def change_password():
    data = request.get_json()
    username = data.get('username')
    password_old = data.get('password_old')
    password_new = data.get('password_new')
    app.logger.info(f"Changing password for {username}: {password_old} -> {password_new}")
    if not all([username, password_old, password_new]):
        return jsonify({"status": "Error", "message": "Missing data"}), 400

    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            cursor.execute("SELECT password_hash FROM account WHERE username = %s", (username,))
            result = cursor.fetchone()
            if result and bcrypt.checkpw(password_old.encode('utf-8'), result['password_hash'].encode('utf-8')):
                hashed_password = bcrypt.hashpw(password_new.encode('utf-8'), bcrypt.gensalt())
                cursor.execute("UPDATE account SET password_hash = %s WHERE username = %s", (hashed_password, username))
                conn.commit()
                return jsonify({"status": "Success"}), 200
            else:
                return jsonify({"status": "Error", "message": "Invalid credentials"}), 401
    except Exception as e:
        return jsonify({"status": "Error", "message": str(e)}), 500
    finally:
        conn.close()

@app.route('/change_username', methods=['POST'])
def change_username():
    data = request.get_json()
    username_old = data.get('username_old')
    password = data.get('password')
    username_new = data.get('username_new')

    if not all([username_old, password, username_new]):
        return jsonify({"status": 1, "message": "Missing data"}), 400

    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            cursor.execute("SELECT password_hash FROM account WHERE username = %s", (username_old,))
            result = cursor.fetchone()
            if result and bcrypt.checkpw(password.encode('utf-8'), result['password_hash'].encode('utf-8')):
                cursor.execute("UPDATE account SET username = %s WHERE username = %s", (username_new, username_old))
                conn.commit()
                return jsonify({"status": 0, "username": username_new}), 200
            else:
                return jsonify({"status": 1, "message": "Invalid credentials"}), 401
    except Exception as e:
        return jsonify({"status": 1, "message": str(e)}), 500
    finally:
        conn.close()


@app.route('/check_like', methods=['GET'])
def check_like():
    username = request.args.get('username')
    song_url = request.args.get('song_url')

    if not all([username, song_url]):
        return jsonify({"exists": False, "message": "Missing data"}), 400

    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            cursor.execute("SELECT id FROM account WHERE username = %s", (username,))
            account = cursor.fetchone()
            if account is None:
                return jsonify({"exists": False, "message": "Account not found"}), 404

            cursor.execute("SELECT id FROM song WHERE url = %s", (song_url,))
            song = cursor.fetchone()
            if song is None:
                return jsonify({"exists": False, "message": "Song not found"}), 404

            cursor.execute("SELECT EXISTS(SELECT account_id, song_id FROM account_song WHERE account_id = %s AND song_id = %s)", 
                           (account['id'], song['id']))
            exists = cursor.fetchone()['EXISTS(SELECT account_id, song_id FROM account_song WHERE account_id = %s AND song_id = %s)']

            return jsonify({"exists": bool(exists)}), 200
    except Exception as e:
        return jsonify({"exists": False, "message": str(e)}), 500
    finally:
        conn.close()


@app.route('/load_album_songs', methods=['GET'])
def load_album_songs():
    album_id = request.args.get('album_id')

    if not album_id:
        return jsonify({"error": "Missing album ID"}), 400

    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            query = """
            SELECT song.id, song.url, song.title, artist.name, album.id, album.url, album.title
            FROM song
            JOIN album ON song.album_id = album.id
            JOIN artist ON album.artist_id = artist.id
            WHERE song.album_id = %s
            ORDER BY song.album_no ASC
            """
            cursor.execute(query, (album_id,))
            results = cursor.fetchall()

            samples = [{
                'song_id': row['id'],
                'song_url': row['url'],
                'song_title': row['title'],
                'artist_name': row['name'],
                'album_id': row['album.id'],
                'album_url': row['album.url'],
                'album_title': row['album.title']
            } for row in results]

            return jsonify(samples), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500
    finally:
        conn.close()


@app.route('/load_albums', methods=['GET'])
def load_albums():
    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            query = "SELECT album.id, album.url, album.title FROM album"
            cursor.execute(query)
            results = cursor.fetchall()

            albums = [{
                'album_id': row['id'],
                'album_url': row['url'],
                'album_title': row['title']
            } for row in results]

            return jsonify(albums), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500
    finally:
        conn.close()


@app.route('/load_library', methods=['GET'])
def load_library():
    username = request.args.get('username')

    if not username:
        return jsonify({"error": "Missing username"}), 400

    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            query = """
            SELECT song.id, song.url, song.title, artist.name, album.id, album.url, album.title
            FROM song
            JOIN album ON song.album_id = album.id
            JOIN artist ON album.artist_id = artist.id
            WHERE song.id IN (
                SELECT song_id 
                FROM account_song 
                WHERE account_id = (
                    SELECT id
                    FROM account
                    WHERE username = %s
                )
            )"""
            cursor.execute(query, (username,))
            results = cursor.fetchall()

            samples = [{
                'song_id': row['id'],
                'song_url': row['url'],
                'song_title': row['title'],
                'artist_name': row['name'],
                'album_id': row['album.id'],
                'album_url': row['album.url'],
                'album_title': row['album.title']
            } for row in results]

            return jsonify(samples), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500
    finally:
        conn.close()


@app.route('/load_playlist_songs', methods=['GET'])
def load_playlist_songs():
    playlist_id = request.args.get('playlist_id')

    if not playlist_id:
        return jsonify({"error": "Missing playlist ID"}), 400

    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            query = """
            SELECT song.id, song.url, song.title, artist.name, album.id, album.url, album.title
            FROM playlist_song
            JOIN song ON playlist_song.song_id = song.id
            JOIN album ON song.album_id = album.id
            JOIN artist ON album.artist_id = artist.id
            WHERE playlist_song.playlist_id = %s
            """
            cursor.execute(query, (playlist_id,))
            results = cursor.fetchall()

            samples = [{
                'song_id': row['id'],
                'song_url': row['url'],
                'song_title': row['title'],
                'artist_name': row['name'],
                'album_id': row['album.id'],
                'album_url': row['album.url'],
                'album_title': row['album.title']
            } for row in results]

            return jsonify(samples), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500
    finally:
        conn.close()

@app.route('/load_playlists', methods=['GET'])
def load_playlists():
    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            query = "SELECT playlist.id, playlist.url, playlist.title FROM playlist"
            cursor.execute(query)
            results = cursor.fetchall()

            playlists = [{
                'playlist_id': row['id'],
                'playlist_url': row['url'],
                'playlist_title': row['title']
            } for row in results]

            return jsonify(playlists), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500
    finally:
        conn.close()

@app.route('/load_songs', methods=['GET'])
def load_songs():
    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            query = """
            SELECT song.id, song.url, song.title, artist.name, album.id, album.url, album.title
            FROM song
            JOIN album ON song.album_id = album.id
            JOIN artist ON album.artist_id = artist.id
            """
            cursor.execute(query)
            results = cursor.fetchall()

            songs = [{
                'song_id': row['id'],
                'song_url': row['url'],
                'song_title': row['title'],
                'artist_name': row['name'],
                'album_id': row['album.id'],
                'album_url': row['album.url'],
                'album_title': row['album.title']
            } for row in results]

            return jsonify(songs), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500
    finally:
        conn.close()

@app.route('/login', methods=['POST'])
def login():
    data = request.get_json()
    username = data.get('username')
    password = data.get('password')

    if not username or not password:
        return jsonify({"username": username, "status": 1}), 400

    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            cursor.execute("SELECT password_hash FROM account WHERE username = %s", (username,))
            result = cursor.fetchone()

            if result and bcrypt.checkpw(password.encode('utf-8'), result['password_hash'].encode('utf-8')):
                status = 0
            else:
                status = 1

            return jsonify({"username": username, "status": status}), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500
    finally:
        conn.close()

@app.route('/remove_like', methods=['POST'])
def remove_like():
    data = request.get_json()
    username = data.get('username')
    song_url = data.get('song_url')

    if not username or not song_url:
        return jsonify({"error": "Missing username or song Google ID"}), 400

    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            cursor.execute("SELECT id FROM account WHERE username = %s", (username,))
            account_result = cursor.fetchone()
            if account_result is None:
                return jsonify({"error": "User not found"}), 404

            cursor.execute("SELECT id FROM song WHERE url = %s", (song_url,))
            song_result = cursor.fetchone()
            if song_result is None:
                return jsonify({"error": "Song not found"}), 404

            delete_query = "DELETE FROM account_song WHERE account_id = %s AND song_id = %s"
            cursor.execute(delete_query, (account_result['id'], song_result['id']))
            conn.commit()

            return jsonify({"message": "Song removed successfully"}), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500
    finally:
        conn.close()

@app.route('/search', methods=['GET'])
def search():
    search_query = request.args.get('q', '')

    if not search_query:
        return jsonify({"error": "Missing search query"}), 400

    search_pattern = f"%{search_query}%"

    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            query = """
            SELECT song.id, song.url, song.title, artist.name, album.id, album.url, album.title
            FROM song
            JOIN album ON song.album_id = album.id
            JOIN artist ON album.artist_id = artist.id
            WHERE song.title LIKE %s OR artist.name LIKE %s OR album.title LIKE %s
            """
            cursor.execute(query, (search_pattern, search_pattern, search_pattern))
            results = cursor.fetchall()

            samples = [{
                'song_id': row['id'],
                'song_url': row['url'],
                'song_title': row['title'],
                'artist_name': row['name'],
                'album_id': row['album.id'],
                'album_url': row['album.url'],
                'album_title': row['album.title']
            } for row in results]

            return jsonify(samples) if samples else jsonify({"message": "No results found"}), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500
    finally:
        conn.close()

@app.route('/signup', methods=['POST'])
def signup():
    data = request.get_json()
    username = data.get('username')
    email = data.get('email')
    password = data.get('password')
    role = data.get('role')

    if not all([username, email, password, role]):
        return jsonify({"username": username, "status": "Error", "message": "Missing data"}), 400

    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            cursor.execute("SELECT email FROM account WHERE username = %s", (username,))
            if cursor.fetchone():
                return jsonify({"username": username, "status": 1}), 409

            cursor.execute("SELECT email FROM account WHERE email = %s", (email,))
            if cursor.fetchone():
                return jsonify({"username": username, "status": 2}), 409

            hashed_password = bcrypt.hashpw(password.encode('utf-8'), bcrypt.gensalt())
            insert_query = """
            INSERT INTO account (username, email, password_hash, role_id)
            VALUES (%s, %s, %s, %s)
            """
            cursor.execute(insert_query, (username, email, hashed_password, role))
            conn.commit()

            return jsonify({"username": username, "status": 0}), 201
    except Exception as e:
        return jsonify({"error": str(e)}), 500
    finally:
        conn.close()

@app.errorhandler(404)
def handle_404(e):
    return redirect(url_for('index'))


@app.errorhandler(500)
def handle_500(e):
    return redirect(url_for('index'))


if __name__ == '__main__':
    serve(app, host='0.0.0.0', port=os.getenv('SERVER_PORT'))