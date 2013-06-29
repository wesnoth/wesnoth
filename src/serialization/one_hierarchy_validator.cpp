#include "serialization/one_hierarchy_validator.hpp"

namespace schema_validation
{

one_hierarchy_validator::one_hierarchy_validator(const std::string & filename)
: schema_validator(filename)
, first_tag_open()
, num_tags_open(0)
{}

void one_hierarchy_validator::open_tag(const std::string & name,
            int start_line,
            const std::string &file,
            bool addition)
{
  if(first_tag_open.empty())
    first_tag_open=name;
  ++num_tags_open;
  schema_validator::open_tag(name, start_line, file, addition);
}

void one_hierarchy_validator::close_tag()
{
  --num_tags_open;
  schema_validator::close_tag();
}

bool one_hierarchy_validator::is_over(const config&) const
{
  return num_tags_open == 0 && !first_tag_open.empty();
}
} // namespace schema_validator