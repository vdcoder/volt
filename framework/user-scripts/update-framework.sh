#!/bin/bash

# ⚡ Volt Framework - Update Framework Script
# Usage: ./update-framework.sh [volt-repo-path]

set -e

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

print_header() {
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}⚡ $1${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
}

print_success() { echo -e "${GREEN}✅ $1${NC}"; }
print_info() { echo -e "${BLUE}ℹ️  $1${NC}"; }
print_warning() { echo -e "${YELLOW}⚠️  $1${NC}"; }
print_error() { echo -e "${RED}❌ $1${NC}"; }

# Show usage
show_usage() {
    cat << EOF
Usage: ./update-framework.sh [volt-repo-path]

Updates the Volt framework in your current project to the latest version.

Arguments:
    volt-repo-path      Path to Volt repository (default: auto-detect from ../volt)

Examples:
    ./update-framework.sh
    ./update-framework.sh ~/projects/volt
    ../volt/user-scripts/update-framework.sh

EOF
}

# Parse arguments
VOLT_REPO="$1"

if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    show_usage
    exit 0
fi

# Auto-detect Volt repository
if [ -z "$VOLT_REPO" ]; then
    # Try common locations
    if [ -d "../volt/volt-client-template" ]; then
        VOLT_REPO="../volt"
    elif [ -d "../../volt/volt-client-template" ]; then
        VOLT_REPO="../../volt"
    else
        print_error "Could not find Volt repository"
        print_info "Please specify the path: ./update-framework.sh <path-to-volt>"
        exit 1
    fi
fi

# Validate paths
TEMPLATE_DIR="$VOLT_REPO/volt-client-template"
FRAMEWORK_SOURCE="$TEMPLATE_DIR/dependencies/volt"

if [ ! -d "$FRAMEWORK_SOURCE" ]; then
    print_error "Framework source not found: $FRAMEWORK_SOURCE"
    print_info "Make sure you're pointing to the Volt repository"
    exit 1
fi

# Check if we're in a Volt app
if [ ! -d "dependencies/volt" ]; then
    print_error "This doesn't appear to be a Volt app directory"
    print_info "Run this script from your app's root directory (where dependencies/ is)"
    exit 1
fi

# Start update process
print_header "Updating Volt Framework"
echo ""

print_info "Volt Repository: $VOLT_REPO"
print_info "Current Directory: $(pwd)"
echo ""

# Backup current framework
print_info "Creating backup..."
BACKUP_DIR="dependencies/volt.backup.$(date +%Y%m%d_%H%M%S)"
cp -r dependencies/volt "$BACKUP_DIR"
print_success "Backup created: $BACKUP_DIR"

# Update framework files
print_info "Updating framework files..."

# Update include files
rm -rf dependencies/volt/include/*
cp -r "$FRAMEWORK_SOURCE/include"/* dependencies/volt/include/
print_success "Updated include files"

# Update source files
rm -rf dependencies/volt/src/*
cp -r "$FRAMEWORK_SOURCE/src"/* dependencies/volt/src/
print_success "Updated source files"

# Show summary
echo ""
print_header "✨ Framework Updated Successfully!"
echo ""
print_success "Your framework is now up to date"
print_info "Backup saved at: $BACKUP_DIR"
echo ""
print_warning "Next steps:"
echo "  1. Review changes (if any breaking changes)"
echo "  2. Rebuild your app: ./build.sh"
echo "  3. Test your app to ensure everything works"
echo "  4. If issues occur, restore backup: rm -rf dependencies/volt && mv $BACKUP_DIR dependencies/volt"
echo ""
