#include "css/computed_style.h"
#include "html/node.h"
#include "css/css_rule.h"
#include "css/cssom.h"

void apply_style(std::shared_ptr<Node> node,CSSOM& cssom, bool is_root = true);
