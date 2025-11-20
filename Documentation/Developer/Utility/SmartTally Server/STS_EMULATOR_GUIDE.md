# STS Emulator User's Guide

## Overview

The STS (Smart Tally Server) Emulator is a comprehensive testing tool for STAC devices that simulates Roland V-60HD and V-160HD video switchers. It provides an interactive, menu-driven interface with powerful error injection capabilities to thoroughly test STAC behavior under various conditions.

**Version:** 1.0.0  
**Python:** 3.13.x  
**Dependencies:** None (uses Python standard library only)

---

## Features

### Core Functionality
- **Dual Protocol Support**: Emulates both V-60HD and V-160HD switchers
- **Multi-STAC Support**: Handle multiple STAC connections simultaneously
- **Auto-Cycling**: Automatically rotate tally states through all channels
- **Manual Control**: Set individual channel states on demand
- **Live Keyboard Control**: Trigger error conditions in real-time while server is running

### Error Injection
- **Response Delays**: Add configurable delays (0-30000ms) to responses
- **Junk Data**: Send random garbage with adjustable probability (0-100%)
- **Ignore Mode**: Stop responding to requests with spacebar trigger
- **Connection Statistics**: Track requests per STAC with detailed metrics

### Logging & Monitoring
- **Timestamped Logs**: Millisecond-precision timestamps on all events
- **Request/Response Logging**: See exact data sent to each STAC
- **Per-STAC Statistics**: Individual counters for each connected device
- **Clean Output**: Auto-cycle events hidden to reduce clutter

---

## Quick Start

### Installation

1. Ensure Python 3.13.x is installed:
   ```bash
   python3 --version
   ```

2. Navigate to the emulator directory:
   ```bash
   cd /path/to/STAC3/Documentation/Developer/Utility/SmartTally\ Server
   ```

3. Run the emulator:
   ```bash
   python3 sts_emulator.py
   ```

### Basic Usage

1. **Configure Settings** (Menu option 1)
   - Port defaults to 8080 (change if needed)
   - Select V-60HD or V-160HD model
   - Set V-160HD credentials if needed

2. **Start Server** (Menu option 4)
   - Server begins listening for STAC connections
   - Auto-cycle starts if enabled
   - Press **ENTER** to stop server

3. **Configure STAC Device**
   - Set STAC to connect to emulator's IP address
   - Use port configured in step 1 (default 8080)
   - STAC will begin polling for tally status

---

## Menu System

### Main Menu

```
======================================================================
 STS EMULATOR - MAIN MENU
======================================================================
 1. Configuration
 2. Tally State Control
 3. Error Injection
 4. Start Server
 5. View Statistics
 0. Exit
======================================================================
```

Navigation:
- Enter the number of your choice and press **ENTER**
- **0** returns to main menu from submenus, or exits from main menu

---

## Configuration Menu

### Settings Overview

**Host IP**: Auto-detected local network IP address  
**Port**: TCP port for server (default: 8080, range: 1-65535)  
**Model**: V-60HD or V-160HD protocol  
**Username/Password**: V-160HD authentication credentials (V-60HD doesn't use authentication)

### Options

**1. Change Port**
- Enter new port number (1-65535)
- Common ports: 8080, 8088, 80
- Avoid ports already in use by other services

**2. Change Model**
- **V-60HD**: Simple protocol, no authentication, channels 1-8
- **V-160HD**: HTTP protocol with auth, HDMI 1-16 + SDI 1-12

**3. Change V-160HD Username/Password** (V-160HD only)
- Default: username (empty), password (empty)
- Must match credentials configured on actual V-160HD switch

---

## Tally State Control

### Auto-Cycle Mode

When enabled, channels automatically rotate through tally states:
```
UNSELECTED → SELECTED → ONAIR → UNSELECTED → ...
```

**Benefits:**
- Continuous testing without manual intervention
- Different channels cycle at different rates (offset timing)
- Simulates realistic switcher behavior

**Cycle Interval**: Time between state changes (default: 10 seconds)

### Manual State Control

Set specific channel states for testing particular scenarios:

**Option 3: Set Channel State Manually**
1. Enter channel number
2. Select state:
   - **1. UNSELECTED**: Camera not selected (gray/blue)
   - **2. SELECTED**: Camera in preview (green)
   - **3. ONAIR**: Camera on program (red)

**Option 4: Reset All Channels**
- Sets all channels to UNSELECTED
- Useful for returning to known state

---

## Error Injection

Simulate various error conditions to test STAC resilience and error handling.

### Response Delay

**Purpose**: Test STAC timeout handling  
**Range**: 0-30000 milliseconds  
**Use Case**: Verify STAC properly handles slow responses

Example: Set 6000ms delay to test timeout behavior (STAC default timeout is ~5000ms)

### Junk Data Probability

**Purpose**: Test protocol error handling  
**Range**: 0-100%  
**Behavior**: Sends random garbage instead of valid tally state

**What STAC Shows:**
- Purple question mark (?) in camera operator mode
- After 8 consecutive junk responses

**Testing Strategy:**
- Set to 25% for intermittent junk responses
- Set to 100% to test continuous protocol errors

### Ignore Request Mode

**Purpose**: Simulate connection loss / no response  
**How It Works:**
1. Set "Ignore Count" (e.g., 8 requests)
2. Start server (option 4)
3. Press **SPACEBAR** to trigger ignore mode
4. Emulator ignores next N requests
5. Automatically returns to normal operation

**What STAC Shows:**
- Displays purple X after 8 consecutive ignored requests
- GROVE port switches to ERROR state at same time

**Live Testing:**
- Press **SPACEBAR** repeatedly to test multiple ignore cycles
- Watch STAC display and GROVE port behavior
- Verify error threshold timing

### Reset All Error Injection

Sets all error injection to defaults:
- Response delay: 0ms
- Junk probability: 0%
- Ignore count: 0
- Ignore trigger: disabled

---

## Running the Server

### Starting the Server

1. Select option **4** from main menu
2. Server starts and displays:
   ```
   ═══════════════════════════════════════════════════════════
   STS Emulator Started
   Model: V-60HD
   Listening on: 192.168.2.58:8080
   Auto-cycle: 10s interval
   ═══════════════════════════════════════════════════════════
   
   Server running.
   Press SPACEBAR to trigger ignore mode, ENTER to stop server...
   ```

### Live Keyboard Controls

While server is running:

**SPACEBAR**: Trigger ignore mode (if ignore count > 0)
- Immediately starts ignoring requests
- Displays confirmation message
- Can be pressed repeatedly for multiple test cycles

**ENTER**: Stop server
- Cleanly shuts down server
- Displays connection statistics
- Returns to main menu

**Note**: Key presses are **not echoed** to the screen while server is running

### Log Output

The emulator displays real-time activity:

```
[15:18:59.534] <-- 192.168.2.27: GET /tally/2/status
[15:18:59.534]    --> 192.168.2.27: selected

[15:19:01.612] <-- 192.168.2.27: GET /tally/2/status
[15:19:01.612]    --> 192.168.2.27: onair
```

**Log Format:**
- `[HH:MM:SS.mmm]` - Timestamp with milliseconds
- `<--` - Request received from STAC
- `-->` - Response sent to STAC
- IP address identifies which STAC (supports multiple connections)
- Actual response text shown (lowercase as sent)

### Ignore Mode Logging

When spacebar is pressed:
```
[SPACEBAR] Ignore mode triggered - will ignore next 8 requests

[15:20:15.234] <-- 192.168.2.27: GET /tally/2/status
[15:20:15.234] --> Ignoring request #1 from 192.168.2.27
[15:20:16.445] <-- 192.168.2.27: GET /tally/2/status
[15:20:16.445] --> Ignoring request #2 from 192.168.2.27
...
[15:20:22.889] <-- 192.168.2.27: GET /tally/2/status
[15:20:22.889] --> Ignoring request #8 from 192.168.2.27
[15:20:22.889] Ignore mode complete (8 requests ignored) - resuming normal operation
```

---

## Connection Statistics

View detailed per-STAC connection statistics (option 5):

```
======================================================================
CONNECTION STATISTICS
======================================================================

STAC: 192.168.2.27
  Total Requests:    245
  Normal Responses:  229
  Delayed Responses: 0
  Junk Responses:    8
  Ignored Requests:  8
  First Request:     15:10:33.421
  Last Request:      15:25:14.567
```

**Metrics:**
- **Total Requests**: All HTTP requests received
- **Normal Responses**: Valid tally states sent
- **Delayed Responses**: Responses sent after delay
- **Junk Responses**: Random garbage sent
- **Ignored Requests**: No response sent (connection closed)
- **First/Last Request**: Timestamps for connection duration

---

## Protocol Details

### V-60HD Protocol

**Request Format:**
```
GET /tally/[1-8]/status
```

**Response Format:** (raw text, no HTTP headers)
```
onair
selected
unselected
```

**Characteristics:**
- Simple text-based protocol
- No authentication
- 8 channels (1-8)
- Fast and lightweight

### V-160HD Protocol

**Request Format:**
```
GET /tally/[bank]_[channel]/status HTTP/1.1
Authorization: Basic <base64(username:password)>
User-Agent: STAC_XXXXX
Connection: keep-alive
```

**Response Format:** (full HTTP response)
```
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 8

selected
```

**Bank/Channel Examples:**
- HDMI inputs: `hdmi_1` through `hdmi_16`
- SDI inputs: `sdi_1` through `sdi_12`

**Characteristics:**
- Full HTTP/1.1 protocol
- Basic authentication required
- Multiple input banks
- More complex than V-60HD

---

## Testing Scenarios

### Scenario 1: Normal Operation Test

**Goal**: Verify STAC correctly displays all tally states

**Setup:**
1. Configuration → Select model matching STAC
2. Tally Control → Enable auto-cycle, 10s interval
3. Start server

**Expected Behavior:**
- STAC cycles through gray → green → red
- Display changes every 10 seconds
- GROVE port outputs correct states

**Pass Criteria:**
- All three states display correctly
- State changes smooth and consistent
- No protocol errors

---

### Scenario 2: Timeout Handling Test

**Goal**: Verify STAC handles response timeouts correctly

**Setup:**
1. Error Injection → Set response delay to 6000ms
2. Start server

**Expected Behavior:**
- First request: STAC waits, then times out
- After 8 timeouts: Purple X appears
- GROVE port switches to ERROR

**Pass Criteria:**
- STAC doesn't crash or freeze
- Display shows purple X after threshold
- GROVE port synchronized with display
- Normal operation resumes when delay removed

---

### Scenario 3: Protocol Error Test

**Goal**: Verify STAC handles garbage responses

**Setup:**
1. Error Injection → Set junk probability to 100%
2. Start server

**Expected Behavior:**
- STAC receives random garbage instead of valid states
- After 8 junk responses: Purple ? appears
- Connection continues (not dropped)

**Pass Criteria:**
- Purple question mark displayed after 8 junk replies
- STAC doesn't crash
- Recovers when junk probability set to 0%

---

### Scenario 4: Connection Loss Test (Live)

**Goal**: Test STAC behavior when server stops responding

**Setup:**
1. Error Injection → Set ignore count to 8
2. Start server
3. Watch STAC display current tally state

**Test Procedure:**
1. Press **SPACEBAR** to trigger ignore mode
2. Watch STAC display - should hold last state
3. Count requests in log (should reach 8)
4. Verify purple X appears on 8th ignore
5. Check GROVE port switches to ERROR simultaneously
6. Press **SPACEBAR** again to test recovery

**Pass Criteria:**
- STAC maintains last tally state for 7 ignores
- Purple X and ERROR GROVE on 8th ignore
- Both display and GROVE synchronized
- Recovers to normal when ignore completes

---

### Scenario 5: Multi-STAC Test

**Goal**: Verify emulator handles multiple STACs correctly

**Setup:**
1. Configure 2+ STAC devices to same emulator IP
2. Tally Control → Enable auto-cycle
3. Start server

**Expected Behavior:**
- Both STACs appear in logs with different IPs
- Each STAC receives independent responses
- Statistics track each STAC separately

**Pass Criteria:**
- All STACs display correct states
- No cross-talk between devices
- Individual statistics for each STAC

---

## Troubleshooting

### Emulator Won't Start

**Error**: `Address already in use`

**Solution**: 
- Change port in Configuration menu
- Or stop other service using that port
- Common conflict: Another emulator or web server

---

### STAC Shows Purple Question Mark

**Cause**: Junk data probability > 0%

**Solution**:
1. Stop server
2. Error Injection → Reset All Error Injection
3. Restart server

**Note**: This is expected behavior when testing junk responses

---

### STAC Not Connecting

**Checklist:**
1. Verify STAC and emulator on same network
2. Check emulator's displayed IP address matches STAC config
3. Verify port number matches (default 8080)
4. Ensure firewall not blocking port
5. Check STAC has valid WiFi connection

**Debug Steps:**
1. Start emulator
2. Note IP address shown: `Listening on: 192.168.2.58:8080`
3. Configure STAC with that exact IP
4. Watch emulator logs for incoming requests

---

### No Logs Appearing

**Cause**: STAC not sending requests

**Solution**:
1. Check STAC WiFi connection status
2. Verify STAC has correct server IP configured
3. Confirm STAC not in provisioning mode
4. Restart STAC device

---

### Keys Not Working While Server Running

**Expected Behavior**: Keys are not echoed to screen in raw mode

**Controls:**
- **SPACEBAR**: Triggers ignore mode (silent, watch for confirmation message)
- **ENTER**: Stops server (may need to press twice)

**Note**: This is intentional - prevents terminal clutter during testing

---

### Statistics Show Zero Requests

**Cause**: No STAC has connected yet

**Solution**:
1. Verify STAC is configured correctly
2. Check network connectivity
3. Wait - statistics update after first request
4. Try accessing emulator from browser: `http://192.168.2.58:8080/tally/1/status`

---

## Tips & Best Practices

### Effective Testing

1. **Start Simple**: Test normal operation before adding error injection
2. **One Variable**: Change one setting at a time
3. **Document Results**: Note STAC behavior for each test scenario
4. **Use Ignore Mode**: Spacebar trigger is fastest way to test connection loss
5. **Watch Logs**: Logs show exactly what's being sent to STAC

### Error Injection Strategy

**For Development:**
- Use low junk probability (10-25%) for intermittent errors
- Use ignore mode with spacebar for repeatable connection tests
- Start with short delays (1000ms) and increase gradually

**For Stress Testing:**
- Combine multiple error types
- Use auto-cycle with high junk probability
- Test extended periods (30+ minutes)

### Multi-STAC Testing

- Use different tally display modes (camera operator vs. talent)
- Test with STACs on different channels
- Verify each STAC maintains independent state
- Check statistics to ensure all STACs receiving responses

### Performance Notes

- Emulator handles 10+ simultaneous STACs easily
- Logs can get busy with multiple devices - watch specific STAC IP
- Statistics help track which STAC has issues
- Clean restart: Stop server, clear stats (restart emulator), start fresh

---

## Advanced Usage

### Testing Specific Sequences

Disable auto-cycle and manually set states to test transitions:

1. Tally Control → Toggle auto-cycle OFF
2. Set channel to UNSELECTED
3. Watch STAC display
4. Set channel to SELECTED
5. Verify smooth transition
6. Set channel to ONAIR
7. Check transition timing

### Timing Analysis

Use log timestamps to measure:
- Request frequency (STAC poll interval)
- Response timing with delays
- Error recovery time
- State change propagation

Example:
```
[15:10:33.534] <-- 192.168.2.27: GET /tally/2/status
[15:10:35.621] <-- 192.168.2.27: GET /tally/2/status
```
Poll interval = 2.087 seconds

### Custom Test Cycles

Create specific test patterns:

1. Set ignore count to specific value
2. Trigger multiple ignore cycles with spacebar
3. Note STAC behavior at each threshold
4. Verify error recovery timing

---

## Version History

### Version 1.0.0
- Initial release
- V-60HD and V-160HD protocol support
- Auto-cycle tally states
- Error injection (delay, junk, ignore)
- Live keyboard controls (spacebar trigger)
- Per-STAC connection statistics
- Simplified navigation (removed ESC key complexity)
- Synchronized response logging
- Terminal cleanup on exit

---

## Support & Feedback

For issues, questions, or suggestions about the STS Emulator:

- Review the STAC v3.0 Developer Guide
- Check the emulator's README.md
- Examine the source code (well-commented Python)
- Test with the original Roland emulator scripts for comparison

---

## Appendix A: Default Settings

| Setting | Default Value | Range |
|---------|--------------|-------|
| Host | Auto-detected | N/A |
| Port | 8080 | 1-65535 |
| Model | V-60HD | V-60HD, V-160HD |
| V-160HD Username | (empty) | Any string |
| V-160HD Password | (empty) | Any string |
| Auto-cycle | Disabled | On/Off |
| Cycle Interval | 10 seconds | > 0 |
| Channel States | All UNSELECTED | UNSELECTED/SELECTED/ONAIR |
| Response Delay | 0 ms | 0-30000 |
| Junk Probability | 0% | 0-100 |
| Ignore Count | 0 | 0+ |

---

## Appendix B: Keyboard Reference

### Main Menu
- `1-5` + ENTER: Select menu option
- `0` + ENTER: Exit from main menu, or back to main menu from submenus

### Server Running
- `SPACEBAR`: Trigger ignore mode (no echo)
- `ENTER`: Stop server

### All Menus
- Numbers correspond to menu options
- Enter required after each selection
- Invalid input shows error message

---

## Appendix C: STAC Tally Display States

| State | Color (Camera Op) | Color (Talent) | GROVE TS_1 | GROVE TS_0 |
|-------|------------------|----------------|-----------|-----------|
| UNSELECTED | Blue/Gray | Blue | LOW | HIGH |
| SELECTED | Green | Green | HIGH | LOW |
| ONAIR | Red | Red | HIGH | HIGH |
| ERROR (No Reply) | Purple X | Green + power | LOW | LOW |
| ERROR (Junk) | Purple ? | Green + power | LOW | LOW |
| ERROR (Timeout) | Orange X | Green + power | LOW | LOW |
| ERROR (Other) | Red X | Green + power | LOW | LOW |

---

*End of STS Emulator User's Guide*
