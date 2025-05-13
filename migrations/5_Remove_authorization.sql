-- We don't need sessions in this backend anymore
drop table session;

-- User IDs are now strings
alter table "user" alter column id type text;
alter table scene alter column author type text;

-- Make sure the scene actually references the user via a FK
-- Not a new design requirement, we just missed it before.
alter table scene
    add constraint fk_author foreign key (author) references "user" (id);