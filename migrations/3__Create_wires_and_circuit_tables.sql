create table circuit
(
    id              uuid primary key not null,
    parent_scene    uuid             not null,
    location_x      int              not null,
    location_y      int              not null,
    parent_circuit  uuid,
    gate_type       text             not null,

    foreign key (parent_scene) references scene (id),
    foreign key (parent_circuit) references circuit (id)
);

create table wire
(
    id              uuid primary key not null,
    source_circuit  uuid             not null,
    target_circuit  uuid             not null,
    init_signal     bool             not null,
    amount_input    int              not null,
    amount_output   int              not null,
    location        text             not null,

    foreign key (source_circuit) references circuit (id),
    foreign key (target_circuit) references circuit (id)
);