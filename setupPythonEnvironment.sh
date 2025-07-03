#!/bin/bash

# DragonRock Python Environment Setup Script
# This script sets up a Python virtual environment and installs required dependencies

set -e  # Exit immediately if a command exits with a non-zero status

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

ENV_DIR="dr_venv"
REQUIREMENTS_FILE="requirements.txt"

echo -e "${BLUE}DragonRock Python Environment Setup${NC}"
echo "----------------------------------------"

# Check if Python is installed
if ! command -v python3 &> /dev/null; then
    echo -e "${RED}Error: Python 3 is not installed or not in PATH${NC}"
    echo "Please install Python 3 and try again"
    exit 1
fi

PYTHON_VERSION=$(python3 --version)
echo -e "${GREEN}Found $PYTHON_VERSION${NC}"

# Check if requirements.txt exists
if [ ! -f "$REQUIREMENTS_FILE" ]; then
    echo -e "${RED}Error: $REQUIREMENTS_FILE not found${NC}"
    echo "Please ensure the file exists in the current directory"
    exit 1
fi

# Check if virtual environment exists
if [ ! -d "$ENV_DIR" ]; then
    echo -e "${YELLOW}Virtual environment not found. Creating...${NC}"
    
    # Check if venv module is available
    if ! python3 -c "import venv" &> /dev/null; then
        echo -e "${RED}Error: Python venv module not available${NC}"
        echo "Please install the Python venv package for your distribution"
        echo "For Ubuntu/Debian: sudo apt-get install python3-venv"
        echo "For Fedora: sudo dnf install python3-libs"
        exit 1
    fi
    
    # Create virtual environment
    python3 -m venv "$ENV_DIR"
    if [ $? -ne 0 ]; then
        echo -e "${RED}Failed to create virtual environment${NC}"
        exit 1
    fi
    echo -e "${GREEN}Virtual environment created successfully${NC}"
else
    echo -e "${GREEN}Found existing virtual environment${NC}"
fi

# Activate virtual environment
echo "Activating virtual environment..."
# Source the activate script
. "$ENV_DIR/bin/activate"

if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to activate virtual environment${NC}"
    exit 1
fi

# Check if activation was successful
if [ -z "$VIRTUAL_ENV" ]; then
    echo -e "${RED}Virtual environment activation failed${NC}"
    exit 1
fi

echo -e "${GREEN}Virtual environment activated successfully${NC}"

# Install requirements
echo "Installing required packages from $REQUIREMENTS_FILE..."
pip install -r "$REQUIREMENTS_FILE"

if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to install packages${NC}"
    exit 1
fi

echo -e "${GREEN}All packages installed successfully${NC}"
echo
echo -e "${BLUE}Python environment is ready!${NC}"
echo "To manually activate this environment later, run:"
echo -e "${YELLOW}source $ENV_DIR/bin/activate${NC}"
echo
echo "When finished, you can deactivate the environment by typing:"
echo -e "${YELLOW}deactivate${NC}"