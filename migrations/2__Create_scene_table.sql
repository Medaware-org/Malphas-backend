create table scene
(
    id          uuid primary key not null,
    author      uuid             not null,
    scene_name  text             not null,
    description text             not null
);