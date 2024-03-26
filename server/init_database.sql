DROP DATABASE music;
CREATE DATABASE music;
USE music;

CREATE TABLE IF NOT EXISTS employee (
    id INT AUTO_INCREMENT,
    name VARCHAR(50),
    salary MEDIUMINT,
    join_date YEAR,
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS label (
    id INT AUTO_INCREMENT,
    title VARCHAR(50),
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS artist (
    id INT AUTO_INCREMENT,
    name VARCHAR(50),
    bio TEXT(1000),
    merch TEXT(1000),
    label_id INT,
    PRIMARY KEY (id),
    FOREIGN KEY (label_id)
        REFERENCES label(id)
        ON DELETE SET NULL
);

CREATE TABLE IF NOT EXISTS album (
    id INT AUTO_INCREMENT,
    url VARCHAR(500),
    title VARCHAR(50),
    artist_id INT,
    release_date YEAR,
    PRIMARY KEY (id),
    FOREIGN KEY (artist_id)
        REFERENCES artist(id)
        ON DELETE SET NULL
);

CREATE TABLE IF NOT EXISTS song (
    id INT AUTO_INCREMENT,
    url VARCHAR(500),
    title VARCHAR(50),
    album_id INT,
    album_no INT,
    PRIMARY KEY (id),
    FOREIGN KEY (album_id)
        REFERENCES album(id)
        ON DELETE SET NULL
);

CREATE TABLE IF NOT EXISTS role (
    id INT AUTO_INCREMENT,
    title VARCHAR(20),
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS account (
    id INT AUTO_INCREMENT,
    username VARCHAR(20),
    email VARCHAR(50),
    password_hash CHAR(60),
    history LONGTEXT,
    role_id INT,
    PRIMARY KEY (id),
    FOREIGN KEY (role_id)
        REFERENCES role(id)
        ON DELETE SET NULL
);

CREATE TABLE IF NOT EXISTS playlist (
    id INT AUTO_INCREMENT,
    url VARCHAR(500),
    title VARCHAR(50),
    account_id INT,
    PRIMARY KEY (id),
    FOREIGN KEY (account_id)
        REFERENCES account(id)
        ON DELETE SET NULL
);

CREATE TABLE IF NOT EXISTS playlist_song (
    playlist_id INT,
    song_id INT,
    PRIMARY KEY (playlist_id, song_id),
    FOREIGN KEY (playlist_id)
        REFERENCES playlist(id),
    FOREIGN KEY (song_id)
        REFERENCES song(id)
);

CREATE TABLE IF NOT EXISTS account_song (
    account_id INT,
    song_id INT,
    PRIMARY KEY (account_id, song_id),
    FOREIGN KEY (account_id)
        REFERENCES account(id),
    FOREIGN KEY (song_id)
        REFERENCES song(id)
);

INSERT INTO role (
    title
)
VALUES
(
    'admin'
),
(
    'user'
),
(
    'artist'
),
(
    'label'
);

INSERT INTO account (
    username,
    password_hash,
    role_id
)
VALUES
(
    'admin',
    '$2y$10$1FVZyP50I7YrEcEcpnAvPOWnIwHXyAsvw3THYhmsEsXKkWOnYHAGa',
    1
);

INSERT INTO artist (
    name
)
VALUES
(
    'Mozart'
),
(
    'Beethoven'
);

INSERT INTO album (
    url,
    title,
    artist_id,
    release_date
)
VALUES
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Mozart/Piano%20Sonata%20No.%2013/cover.jpg',
    'Piano Sonata No. 13',
    (SELECT id FROM artist WHERE name = 'Mozart'),
    2007
),
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Mozart/Symphony%20No.%2040/cover.jpg',
    'Symphony No. 40',
    (SELECT id FROM artist WHERE name = 'Mozart'),
    2007
),
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Beethoven/Piano%20Sonata%20No.%201/cover.jpg',
    'Piano Sonata No. 1',
    (SELECT id FROM artist WHERE name = 'Beethoven'),
    2007
);

INSERT INTO song (
    url,
    title,
    album_id,
    album_no
)

VALUES 
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Mozart/Piano%20Sonata%20No.%2013/I_Allegro.mp3',
    'I Allegro',
    (SELECT id FROM album WHERE title = 'Piano Sonata No. 13'),
    1
),
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Mozart/Piano%20Sonata%20No.%2013/II_Andante_Cantabile.mp3',
    'II Andante Cantabile',
    (SELECT id FROM album WHERE title = 'Piano Sonata No. 13'),
    2
),
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Mozart/Symphony%20No.%2040/I%20Molto%20Allegro.mp3',
    'I Molto Allegro',
    (SELECT id FROM album WHERE title = 'Symphony No. 40'),
    1
),
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Beethoven/Piano%20Sonata%20No.%201/I%20Allegro.mp3',
    'I Allegro',
    (SELECT id FROM album WHERE title = 'Piano Sonata No. 1'),
    1
),
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Beethoven/Piano%20Sonata%20No.%201/II%20Adagio.mp3',
    'II Adagio',
    (SELECT id FROM album WHERE title = 'Piano Sonata No. 1'),
    2
),
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Beethoven/Piano%20Sonata%20No.%201/III%20Menuetto%20Allegretto.mp3',
    'III Menuetto Allegretto',
    (SELECT id FROM album WHERE title = 'Piano Sonata No. 1'),
    3
),
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Beethoven/Piano%20Sonata%20No.%201/IV%20Prestissimo.mp3',
    'IV Prestissimo',
    (SELECT id FROM album WHERE title = 'Piano Sonata No. 1'),
    4
);

INSERT INTO playlist (
    url,
    title
)
VALUES 
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Playlists/playlist10.jpg',
    'Mix #1'    
),
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Playlists/playlist11.jpg',
    'Mix #2'
),
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Playlists/playlist16.jpg',
    'Mix #3'
),
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Playlists/playlist15.jpg',
    'Mix #4'
),
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Playlists/playlist13.jpg',
    'Mix #5'
),
(
    'https://storage.googleapis.com/musicstreamingqt-songs/Playlists/playlist19.jpg',
    'Mix #6'
);

INSERT INTO playlist_song (
    playlist_id,
    song_id
)
VALUES
(
    (SELECT id FROM playlist WHERE title = 'Mix #1'),
    (SELECT id FROM song WHERE title = 'I Allegro' AND album_id = (SELECT id FROM album WHERE title = 'Piano Sonata No. 13'))
),
(
    (SELECT id FROM playlist WHERE title = 'Mix #1'),
    (SELECT id FROM song WHERE title = 'II Andante Cantabile' AND album_id = (SELECT id FROM album WHERE title = 'Piano Sonata No. 13'))
),
(
    (SELECT id FROM playlist WHERE title = 'Mix #2'),
    (SELECT id FROM song WHERE title = 'II Andante Cantabile' AND album_id = (SELECT id FROM album WHERE title = 'Piano Sonata No. 13'))
),
(
    (SELECT id FROM playlist WHERE title = 'Mix #2'),
    (SELECT id FROM song WHERE title = 'I Molto Allegro' AND album_id = (SELECT id FROM album WHERE title = 'Symphony No. 40'))
),
(
    (SELECT id FROM playlist WHERE title = 'Mix #3'),
    (SELECT id FROM song WHERE title = 'I Allegro' AND album_id = (SELECT id FROM album WHERE title = 'Piano Sonata No. 1'))
),
(
    (SELECT id FROM playlist WHERE title = 'Mix #3'),
    (SELECT id FROM song WHERE title = 'I Molto Allegro' AND album_id = (SELECT id FROM album WHERE title = 'Symphony No. 40'))
),
(
    (SELECT id FROM playlist WHERE title = 'Mix #4'),
    (SELECT id FROM song WHERE title = 'I Allegro' AND album_id = (SELECT id FROM album WHERE title = 'Piano Sonata No. 1'))
),
(
    (SELECT id FROM playlist WHERE title = 'Mix #4'),
    (SELECT id FROM song WHERE title = 'I Molto Allegro' AND album_id = (SELECT id FROM album WHERE title = 'Symphony No. 40'))
),
(
    (SELECT id FROM playlist WHERE title = 'Mix #5'),
    (SELECT id FROM song WHERE title = 'I Allegro' AND album_id = (SELECT id FROM album WHERE title = 'Piano Sonata No. 1'))
),
(
    (SELECT id FROM playlist WHERE title = 'Mix #5'),
    (SELECT id FROM song WHERE title = 'I Molto Allegro' AND album_id = (SELECT id FROM album WHERE title = 'Symphony No. 40'))
),
(
    (SELECT id FROM playlist WHERE title = 'Mix #6'),
    (SELECT id FROM song WHERE title = 'I Allegro' AND album_id = (SELECT id FROM album WHERE title = 'Piano Sonata No. 1'))
),
(
    (SELECT id FROM playlist WHERE title = 'Mix #6'),
    (SELECT id FROM song WHERE title = 'I Molto Allegro' AND album_id = (SELECT id FROM album WHERE title = 'Symphony No. 40'))
);