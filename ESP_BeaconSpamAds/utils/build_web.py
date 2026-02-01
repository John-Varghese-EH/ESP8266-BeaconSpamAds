#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ESP Beacon Spam Web Builder
==============================
Converts web source files (HTML, CSS, JS) from web/ folder into a single
embedded PROGMEM header file (web_index.h) for the ESP.

Usage:
    python utils/build_web.py
"""

import os
import sys
import re
from pathlib import Path
from datetime import datetime

def get_script_dir():
    return Path(__file__).parent.absolute()

def get_project_root():
    return get_script_dir().parent

def minify_css(css: str) -> str:
    css = re.sub(r'/\*[\s\S]*?\*/', '', css)
    css = re.sub(r'\s+', ' ', css)
    css = re.sub(r'\s*([{};:,>~+])\s*', r'\1', css)
    css = re.sub(r';\s*}', '}', css)
    return css.strip()

def minify_js(js: str) -> str:
    # Very basic JS minification
    lines = js.split('\n')
    result = []
    for line in lines:
        line = line.strip()
        if not line: continue
        if line.startswith('//'): continue
        result.append(line)
    return ' '.join(result)

def minify_html(html: str) -> str:
    html = re.sub(r'<!--(?!\[if).*?-->', '', html, flags=re.DOTALL)
    html = re.sub(r'>\s+<', '><', html)
    html = re.sub(r'\s+', ' ', html)
    return html.strip()

def read_file(filepath: Path) -> str:
    with open(filepath, 'r', encoding='utf-8') as f:
        return f.read()

def inline_resources(html: str, base_dir: Path) -> str:
    # Inline CSS
    def replace_css(match):
        css_file = match.group(1)
        # Try finding in the same directory as the HTML file first
        css_path = base_dir / css_file
        if css_path.exists():
            print(f"  [+] Inlining: {css_file}")
            return f"<style>{minify_css(read_file(css_path))}</style>"
        return match.group(0)
    
    html = re.sub(r'<link\s+[^>]*href=["\']([^"\']+\.css)["\'][^>]*>', 
                  replace_css, html, flags=re.IGNORECASE)
    
    # Inline JS
    def replace_js(match):
        js_file = match.group(1)
        js_path = base_dir / js_file
        if js_path.exists():
            print(f"  [+] Inlining: {js_file}")
            return f"<script>{minify_js(read_file(js_path))}</script>"
        return match.group(0)
    
    html = re.sub(r'<script\s+[^>]*src=["\']([^"\']+\.js)["\'][^>]*>\s*</script>', 
                  replace_js, html, flags=re.IGNORECASE)
    
    return html

def generate_header(admin_html: str, portal_html: str) -> str:
    ts = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    return f'''#pragma once

// ESP Beacon Spam Web Interface - Auto-generated
// Built: {ts}
// Edit files in web/ then run: python utils/build_web.py

const char admin_html[] PROGMEM = R"rawliteral({admin_html})rawliteral";
const char portal_html[] PROGMEM = R"rawliteral({portal_html})rawliteral";
'''

def main():
    root = get_project_root()
    web_dir = root / "web"
    admin_dir = web_dir / "admin"
    out_file = root / "web_index.h"
    
    if not web_dir.exists():
        print(f"[ERROR] Web directory not found: {web_dir}")
        sys.exit(1)
        
    print(f"Building web assets from {web_dir}...")
    
    # Process Admin Page
    print("Processing Admin Page (admin/index.html)...")
    if (admin_dir / "index.html").exists():
        admin_html = read_file(admin_dir / "index.html")
        # Pass admin_dir so it finds style.css/app.js in there
        admin_html = inline_resources(admin_html, admin_dir)
        admin_html = minify_html(admin_html)
    else:
        print("[ERROR] admin/index.html not found!")
        sys.exit(1)
    
    # Process Portal Page
    print("Processing Portal Page (portal.html)...")
    if (web_dir / "portal.html").exists():
        portal_html = read_file(web_dir / "portal.html")
        portal_html = inline_resources(portal_html, web_dir) 
        portal_html = minify_html(portal_html)
    else:
         print("[ERROR] portal.html not found!")
         sys.exit(1)
    
    # Write Header
    with open(out_file, 'w', encoding='utf-8') as f:
        f.write(generate_header(admin_html, portal_html))
        
    print(f"Success! Generated {out_file}")

if __name__ == "__main__":
    main()
