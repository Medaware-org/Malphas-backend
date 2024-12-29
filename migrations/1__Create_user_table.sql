create table "user"
(
    id          uuid primary key not null,
    nickname    text unique      not null,
    passwd_hash text             not null
);

create table session
(
    session_token text not null primary key,
    user_id       uuid not null,

    foreign key (user_id) references "user" (id)
);