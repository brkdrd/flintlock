#!/usr/bin/env bash
set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

INSTALL_DIR="$HOME/.local/share/godot-mcp"
NODE_VERSION="22"

print_step() { echo -e "\n${CYAN}[$1/${TOTAL_STEPS}]${NC} $2"; }
print_ok()   { echo -e "    ${GREEN}OK${NC} $1"; }
print_warn() { echo -e "    ${YELLOW}WARNING${NC} $1"; }
print_err()  { echo -e "    ${RED}ERROR${NC} $1"; }

TOTAL_STEPS=6

echo -e "${CYAN}"
echo "  =========================================="
echo "   Godot MCP Installer (satelliteoflove)"
echo "   For Claude Code on WSL2"
echo "  =========================================="
echo -e "${NC}"

# ── 1. Check / install nvm ──────────────────────────────────────────────────

print_step 1 "Checking for nvm..."

export NVM_DIR="${NVM_DIR:-$HOME/.nvm}"
if [ -s "$NVM_DIR/nvm.sh" ]; then
    . "$NVM_DIR/nvm.sh"
fi

if command -v nvm &>/dev/null; then
    print_ok "nvm is already installed ($(nvm --version))"
else
    echo "    nvm not found. Installing..."
    curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.3/install.sh | bash
    export NVM_DIR="$HOME/.nvm"
    . "$NVM_DIR/nvm.sh"
    if command -v nvm &>/dev/null; then
        print_ok "nvm installed ($(nvm --version))"
    else
        print_err "nvm installation failed. Please install it manually:"
        echo "    https://github.com/nvm-sh/nvm#installing-and-updating"
        exit 1
    fi
fi

# ── 2. Check / install Node.js ──────────────────────────────────────────────

print_step 2 "Checking for Node.js >= 18..."

install_node() {
    echo "    Installing Node.js v${NODE_VERSION} via nvm..."
    nvm install "$NODE_VERSION"
    nvm use "$NODE_VERSION"
    nvm alias default "$NODE_VERSION"
}

if command -v node &>/dev/null; then
    CURRENT_NODE_MAJOR=$(node -v | sed 's/v//' | cut -d. -f1)
    if [ "$CURRENT_NODE_MAJOR" -ge 18 ]; then
        print_ok "Node.js $(node -v) is installed"
    else
        print_warn "Node.js $(node -v) is too old (need >= 18)"
        install_node
    fi
else
    echo "    Node.js not found."
    install_node
fi

if ! command -v npm &>/dev/null; then
    print_err "npm not found even after Node.js install. Something went wrong."
    exit 1
fi
print_ok "npm $(npm -v) is available"

# ── 3. Check for git ────────────────────────────────────────────────────────

print_step 3 "Checking for git..."

if command -v git &>/dev/null; then
    print_ok "git is installed ($(git --version | awk '{print $3}'))"
else
    echo "    git not found. Installing via apt..."
    sudo apt-get update -qq && sudo apt-get install -y -qq git
    if command -v git &>/dev/null; then
        print_ok "git installed"
    else
        print_err "Could not install git. Please install it manually."
        exit 1
    fi
fi

# ── 4. Clone and build godot-mcp ────────────────────────────────────────────

print_step 4 "Cloning and building godot-mcp..."

if [ -d "$INSTALL_DIR" ]; then
    echo "    Existing installation found at $INSTALL_DIR"
    echo "    Pulling latest changes..."
    cd "$INSTALL_DIR"
    git pull --ff-only || {
        print_warn "git pull failed, doing a fresh clone instead"
        cd "$HOME"
        rm -rf "$INSTALL_DIR"
        git clone https://github.com/satelliteoflove/godot-mcp.git "$INSTALL_DIR"
        cd "$INSTALL_DIR"
    }
else
    git clone https://github.com/satelliteoflove/godot-mcp.git "$INSTALL_DIR"
    cd "$INSTALL_DIR"
fi

echo "    Installing dependencies..."
cd server
npm install --loglevel=warn
echo "    Building..."
npm run build
cd "$INSTALL_DIR"
print_ok "godot-mcp built at $INSTALL_DIR"

# ── 5. Register with Claude Code ────────────────────────────────────────────

print_step 5 "Registering MCP server with Claude Code..."

SERVER_ENTRY="$INSTALL_DIR/server/dist/index.js"

if command -v claude &>/dev/null; then
    claude mcp remove godot-mcp 2>/dev/null || true
    claude mcp add --transport stdio godot-mcp -- node "$SERVER_ENTRY"
    print_ok "MCP server registered with Claude Code"
else
    print_warn "claude CLI not found in PATH. Skipping auto-registration."
    echo ""
    echo "    To register manually later, run:"
    echo "    claude mcp add --transport stdio godot-mcp -- node $SERVER_ENTRY"
    echo ""
    echo "    Or add this to your ~/.claude.json manually:"
    echo "    {"
    echo "      \"mcpServers\": {"
    echo "        \"godot-mcp\": {"
    echo "          \"type\": \"stdio\","
    echo "          \"command\": \"node\","
    echo "          \"args\": [\"$SERVER_ENTRY\"]"
    echo "        }"
    echo "      }"
    echo "    }"
fi

# ── 6. Print next steps ─────────────────────────────────────────────────────

print_step 6 "Done!"

ADDON_SOURCE="$INSTALL_DIR/godot/addons/godot_mcp"

echo ""
echo -e "${GREEN}  Installation complete!${NC}"
echo ""
echo -e "  ${YELLOW}What you still need to do:${NC}"
echo ""
echo "  1. Copy the Godot addon into your project:"
echo ""
echo -e "     ${CYAN}cp -r $ADDON_SOURCE /path/to/your/godot/project/addons/${NC}"
echo ""
echo "  2. Open your project in Godot (on Windows)."
echo "     Go to Project > Project Settings > Plugins"
echo "     Enable \"Godot MCP\"."
echo ""
echo "  3. Restart Claude Code and verify with /mcp"
echo ""
echo -e "  ${YELLOW}WSL2 note:${NC} The server auto-detects the Windows host IP."
echo "  If it can't connect, set GODOT_HOST and GODOT_PORT:"
echo ""
echo "     claude mcp remove godot-mcp"
echo "     claude mcp add --transport stdio -e GODOT_HOST=\$(cat /etc/resolv.conf | grep nameserver | awk '{print \$2}') -e GODOT_PORT=6550 godot-mcp -- node $SERVER_ENTRY"
echo ""
echo "  Or enable mirrored networking in C:\\Users\\<You>\\.wslconfig:"
echo "     [wsl2]"
echo "     networkingMode=mirrored"
echo ""