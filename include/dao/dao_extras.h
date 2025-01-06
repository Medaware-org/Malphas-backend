#pragma once

#include "dao.h"

[[nodiscard]] inline bool get_wires_in_scene(Database &db, std::vector<wire> &dst, const std::string &scene)
{
        return get_wires_in_scene(db, dst, scene, scene);
}
