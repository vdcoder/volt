#!/bin/bash

# ⚡ Volt Framework - Create New App Script
# Usage: ./create-volt-app.sh <app-name> [options]

set -e

# Colors for pretty output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TEMPLATE_BASE_DIR="$SCRIPT_DIR/../../"

# Function to print colored output
print_header() {
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}⚡ $1${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

# Show usage
show_usage() {
    cat << EOF
Usage: ./create-volt-app.sh <app-name> [options]

Creates a new Volt application from a template.

Options:
    --guid <guid>          Set VOLT_GUID for this app (default: app-name)
    --output <dir>         Output directory (default: ../app-name)
    --template <name>      Template to use (default: x)
                           Available:
                             raw      -> app-template
                             x        -> app-template-x
    --no-git               Don't initialize git repository
    --help, -h             Show this help message

Examples:
    ./create-volt-app.sh my-awesome-app
    ./create-volt-app.sh my-app --guid "myapp_v1"
    ./create-volt-app.sh my-app --template default
    ./create-volt-app.sh my-app --output ~/projects/my-app

EOF
}

# Parse arguments
APP_NAME=""
APP_GUID=""
OUTPUT_DIR=""
INIT_GIT=true
# default to X template now
TEMPLATE_VARIANT="x"

while [[ $# -gt 0 ]]; do
    case $1 in
        --guid)
            APP_GUID="$2"
            shift 2
            ;;
        --output)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        --template)
            TEMPLATE_VARIANT="$2"
            shift 2
            ;;
        --no-git)
            INIT_GIT=false
            shift
            ;;
        --help|-h)
            show_usage
            exit 0
            ;;
        *)
            if [ -z "$APP_NAME" ]; then
                APP_NAME="$1"
                shift
            else
                print_error "Unknown argument: $1"
                show_usage
                exit 1
            fi
            ;;
    esac
done

# Validate app name
if [ -z "$APP_NAME" ]; then
    print_error "App name is required"
    show_usage
    exit 1
fi

# Set defaults
if [ -z "$APP_GUID" ]; then
    APP_GUID="$APP_NAME"
fi

if [ -z "$OUTPUT_DIR" ]; then
    OUTPUT_DIR="$SCRIPT_DIR/../../../$APP_NAME"
fi

# Resolve template directory and template-specific filenames
case "$TEMPLATE_VARIANT" in
    raw)
        TEMPLATE_DIR="${TEMPLATE_BASE_DIR}app-template"
        TEMPLATE_LABEL="raw (app-template)"
        APP_HEADER="App.hpp"
        MAIN_CPP="main.cpp"
        ;;
    x)
        TEMPLATE_DIR="${TEMPLATE_BASE_DIR}app-template-x"
        TEMPLATE_LABEL="x (app-template-x)"
        APP_HEADER="App.x.hpp"
        MAIN_CPP="main.x.cpp"
        ;;
    *)
        print_error "Unknown template: $TEMPLATE_VARIANT"
        print_info "Valid templates: raw, x"
        exit 1
        ;;
esac

# Check if template exists
if [ ! -d "$TEMPLATE_DIR" ]; then
    print_error "Template directory not found: $TEMPLATE_DIR"
    print_info "Make sure you're running this script from the volt repository"
    exit 1
fi

# Check if output directory already exists
if [ -d "$OUTPUT_DIR" ]; then
    print_error "Directory already exists: $OUTPUT_DIR"
    print_warning "Choose a different name or remove the existing directory"
    exit 1
fi

# Start creation process
print_header "Creating Volt App: $APP_NAME"
echo ""

print_info "App Name: $APP_NAME"
print_info "GUID: $APP_GUID"
print_info "Template: $TEMPLATE_LABEL"
print_info "Output Directory: $OUTPUT_DIR"
echo ""

# Copy template
print_info "Copying template files..."
cp -r "$TEMPLATE_DIR" "$OUTPUT_DIR"
print_success "Template copied"

# Copy framework
print_info "Copying framework files..."
cp -r "$SCRIPT_DIR/../../framework/include" "$OUTPUT_DIR/dependencies/volt"
print_success "Framework copied"

# Customize app files with app-specific names
print_info "Customizing app files..."
APP_CLASS_NAME="$(echo $APP_NAME | sed 's/-/_/g' | awk '{for(i=1;i<=NF;i++) $i=toupper(substr($i,1,1)) tolower(substr($i,2))}1' FS='_' OFS='')"
APP_NAME_UNDERSCORE="${APP_NAME//-/_}"

APP_HEADER_PATH="$OUTPUT_DIR/src/$APP_HEADER"
MAIN_CPP_PATH="$OUTPUT_DIR/src/$MAIN_CPP"

# Update App header: Replace tokens
if [ -f "$APP_HEADER_PATH" ]; then
    sed -i "s/VOLT_APP_NAME_CAMEL/${APP_CLASS_NAME}/g" "$APP_HEADER_PATH" 2>/dev/null || \
        sed -i '' "s/VOLT_APP_NAME_CAMEL/${APP_CLASS_NAME}/g" "$APP_HEADER_PATH"

    sed -i "s/VOLT_APP_NAME/${APP_NAME}/g" "$APP_HEADER_PATH" 2>/dev/null || \
        sed -i '' "s/VOLT_APP_NAME/${APP_NAME}/g" "$APP_HEADER_PATH"
else
    print_warning "App header file not found: $APP_HEADER_PATH"
fi

# Update main.cpp: Replace tokens
if [ -f "$MAIN_CPP_PATH" ]; then
    sed -i "s/VOLT_APP_NAME_CAMEL/${APP_CLASS_NAME}/g" "$MAIN_CPP_PATH" 2>/dev/null || \
        sed -i '' "s/VOLT_APP_NAME_CAMEL/${APP_CLASS_NAME}/g" "$MAIN_CPP_PATH"

    sed -i "s/VOLT_APP_NAME_UNDERSCORE/${APP_NAME_UNDERSCORE}/g" "$MAIN_CPP_PATH" 2>/dev/null || \
        sed -i '' "s/VOLT_APP_NAME_UNDERSCORE/${APP_NAME_UNDERSCORE}/g" "$MAIN_CPP_PATH"
else
    print_warning "Main file not found: $MAIN_CPP_PATH"
fi

# Update index.html with app name
if [ -f "$OUTPUT_DIR/index.html" ]; then
    sed -i "s/Volt App/${APP_NAME}/g" "$OUTPUT_DIR/index.html" 2>/dev/null || \
        sed -i '' "s/Volt App/${APP_NAME}/g" "$OUTPUT_DIR/index.html"
fi

# Update build.sh with GUID
if [ -f "$OUTPUT_DIR/build.sh" ]; then
    sed -i "s/GUID=\"\${VOLT_GUID:-demo}\"/GUID=\"\${VOLT_GUID:-${APP_GUID}}\"/g" "$OUTPUT_DIR/build.sh" 2>/dev/null || \
        sed -i '' "s/GUID=\"\${VOLT_GUID:-demo}\"/GUID=\"\${VOLT_GUID:-${APP_GUID}}\"/g" "$OUTPUT_DIR/build.sh"
fi

print_success "Files customized"

# Make scripts executable
if [ -f "$OUTPUT_DIR/build.sh" ]; then
    chmod +x "$OUTPUT_DIR/build.sh"
fi
if [ -f "$OUTPUT_DIR/setup.sh" ]; then
    chmod +x "$OUTPUT_DIR/setup.sh"
fi
if [ -f "$OUTPUT_DIR/deploy.sh" ]; then
    chmod +x "$OUTPUT_DIR/deploy.sh"
fi
print_success "Scripts made executable"

# Initialize git repository
if [ "$INIT_GIT" = true ]; then
    print_info "Initializing git repository..."
    cd "$OUTPUT_DIR"
    git init > /dev/null 2>&1
    git add . > /dev/null 2>&1
    git commit -m "Initial commit - Created with Volt Framework" > /dev/null 2>&1
    print_success "Git repository initialized"
fi

# Print success message
echo ""
print_header "🎉 App Created Successfully!"
echo ""
print_success "Your Volt app is ready at: $OUTPUT_DIR"
echo ""
print_info "Next steps:"
echo ""
echo "  cd $OUTPUT_DIR"
echo "  ./build.sh              # Build the app"
echo "  cd output"
echo "  python3 -m http.server 8001"
echo "  # Open http://localhost:8001"
echo ""
print_info "Useful commands:"
echo ""
echo "  ./setup.sh             # Setup dev environment (first time)"
echo "  ./deploy.sh            # Deploy to Railway"
echo ""
print_info "GUID: $APP_GUID"
print_warning "You can safely delete the volt/ directory if you don't need to create more apps"
echo ""
print_header "Happy coding! ⚡"
echo ""
