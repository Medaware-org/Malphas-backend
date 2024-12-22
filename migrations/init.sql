create extension if not exists "uuid-ossp";

-- Keep track of the db migrations that we already ran
create table db_migration
(
    number int primary key
);