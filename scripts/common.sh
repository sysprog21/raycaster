# Common utilities for git hooks
# Shared color definitions and helper functions

RED=""
GREEN=""
YELLOW=""
BLUE=""
WHITE=""
CYAN=""
NC=""

set_colors() {
  local default_color
  default_color=$(git config --get color.ui || echo 'auto')
  if [[ "$default_color" == "always" ]] || [[ "$default_color" == "auto" && -t 1 ]]; then
    RED='\033[1;31m'
    GREEN='\033[1;32m'
    YELLOW='\033[1;33m'
    BLUE='\033[1;34m'
    WHITE='\033[1;37m'
    CYAN='\033[1;36m'
    NC='\033[0m'
  fi
}

# Print error message and exit
throw() {
  local fmt="$1"
  shift
  printf "\n${RED}[!] $fmt${NC}\n" "$@" >&2
  exit 1
}

# Check if running in CI environment
check_ci_environment() {
  if [ -d "/home/runner/work" ] || [ -n "$CI" ] || [ -n "$GITHUB_ACTIONS" ]; then
    return 0
  fi
  return 1
}

# Get list of git-tracked C/C++ files (NUL-delimited for safe handling)
get_tracked_c_files() {
  git ls-files -z -- '*.c' '*.h' '*.cpp' '*.hpp' ':!:externals/*' 2>/dev/null
}

# Get list of staged C/C++ files (NUL-delimited for safe handling)
get_staged_c_files() {
  git diff --cached --name-only -z --diff-filter=ACMR -- '*.c' '*.cpp' '*.h' '*.hpp' 2>/dev/null
}
