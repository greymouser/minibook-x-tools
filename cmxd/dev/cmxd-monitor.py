#!/usr/bin/env python3
"""
CMXD Event Monitor

A simple client for monitoring events from the CMXD Unix domain socket.
Connects to the socket and displays all events in real-time until Ctrl+C.

Usage:
    ./cmxd-monitor.py [socket_path]

Default socket path: /run/cmxd/events.sock
"""

import socket
import sys
import json
import time
import os
import signal

# Default socket paths
DEFAULT_SOCKET_PATH = "/run/cmxd/events.sock"
TEST_SOCKET_PATH = "/tmp/cmxd/events.sock"

class CMXDMonitor:
    def __init__(self, socket_path=None):
        self.socket_path = socket_path or self.detect_socket_path()
        self.client_socket = None
        self.running = True
        
        # Set up signal handler for graceful shutdown
        signal.signal(signal.SIGINT, self.signal_handler)
        signal.signal(signal.SIGTERM, self.signal_handler)
    
    def detect_socket_path(self):
        """Detect which socket path to use"""
        if os.path.exists(DEFAULT_SOCKET_PATH):
            return DEFAULT_SOCKET_PATH
        elif os.path.exists(TEST_SOCKET_PATH):
            return TEST_SOCKET_PATH
        else:
            # Default to production path
            return DEFAULT_SOCKET_PATH
    
    def signal_handler(self, signum, frame):
        """Handle Ctrl+C gracefully"""
        print(f"\n[SIGNAL] Received signal {signum}, shutting down...")
        self.running = False
        if self.client_socket:
            self.client_socket.close()
    
    def format_timestamp(self, timestamp):
        """Format timestamp for display"""
        if isinstance(timestamp, str):
            try:
                timestamp = float(timestamp)
            except ValueError:
                return timestamp
        
        return time.strftime('%H:%M:%S', time.localtime(timestamp))
    
    def format_event(self, event):
        """Format event for display"""
        try:
            timestamp = self.format_timestamp(event.get('timestamp', time.time()))
            event_type = event.get('type', 'unknown').upper()
            value = event.get('value', '')
            previous = event.get('previous', None)
            
            if previous:
                return f"[{timestamp}] {event_type}: {previous} → {value}"
            else:
                return f"[{timestamp}] {event_type}: {value}"
                
        except Exception as e:
            return f"[{time.strftime('%H:%M:%S')}] RAW: {event} (parse error: {e})"
    
    def connect(self):
        """Connect to the CMXD socket"""
        try:
            self.client_socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            self.client_socket.connect(self.socket_path)
            return True
        except ConnectionRefusedError:
            print(f"✗ Connection refused: {self.socket_path}")
            print("  Make sure cmxd is running with Unix domain socket enabled")
            return False
        except FileNotFoundError:
            print(f"✗ Socket not found: {self.socket_path}")
            print("  Make sure cmxd is running and the socket path is correct")
            return False
        except PermissionError:
            print(f"✗ Permission denied: {self.socket_path}")
            print("  Check socket permissions or run as appropriate user")
            return False
        except Exception as e:
            print(f"✗ Connection error: {e}")
            return False
    
    def run(self):
        """Main monitoring loop"""
        print("CMXD Event Monitor")
        print(f"Socket: {self.socket_path}")
        print("=" * 50)
        
        if not self.connect():
            return 1
        
        print("✓ Connected to CMXD socket")
        print("Monitoring events... (Press Ctrl+C to exit)")
        print()
        
        buffer = ""
        try:
            while self.running:
                try:
                    # Receive data
                    data = self.client_socket.recv(1024).decode('utf-8')
                    if not data:
                        print("[DISCONNECT] Server closed connection")
                        break
                    
                    buffer += data
                    
                    # Process complete JSON lines
                    while '\n' in buffer:
                        line, buffer = buffer.split('\n', 1)
                        if line.strip():
                            try:
                                event = json.loads(line.strip())
                                print(self.format_event(event))
                            except json.JSONDecodeError as e:
                                print(f"[ERROR] Invalid JSON: {line.strip()}")
                                print(f"        Parse error: {e}")
                                
                except socket.timeout:
                    continue
                except socket.error as e:
                    if self.running:  # Only show error if we're not shutting down
                        print(f"[ERROR] Socket error: {e}")
                    break
                except KeyboardInterrupt:
                    break
                    
        except Exception as e:
            print(f"[ERROR] Unexpected error: {e}")
            return 1
        finally:
            if self.client_socket:
                self.client_socket.close()
        
        print("\n✓ Disconnected from CMXD socket")
        return 0

def main():
    """Main entry point"""
    socket_path = None
    
    # Parse command line arguments
    if len(sys.argv) > 1:
        if sys.argv[1] in ['-h', '--help']:
            print(__doc__)
            return 0
        socket_path = sys.argv[1]
    
    # Create and run monitor
    monitor = CMXDMonitor(socket_path)
    return monitor.run()

if __name__ == "__main__":
    sys.exit(main())