 /**
* Copyright (c) 2015-2017 Parity Technologies (UK) Ltd.
* Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
*
* This file is part of metaverse-mvstray.
*
* metaverse-explorer is free software: you can redistribute it and/or
* modify it under the terms of the GNU Affero General Public License with
* additional permissions to the one published by the Free Software
* Foundation, either version 3 of the License, or (at your option)
* any later version. For more information see LICENSE.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

import Cocoa

@NSApplicationMain
@available(macOS, deprecated: 10.11)

class AppDelegate: NSObject, NSApplicationDelegate {
	@IBOutlet weak var statusMenu: NSMenu!
	@IBOutlet weak var startAtLogonMenuItem: NSMenuItem!
	
	let statusItem = NSStatusBar.system().statusItem(withLength: NSVariableStatusItemLength)
	var metaversePid: Int32? = nil
	var commandLine: [String] = []
	let defaultDefaults = "{\"fat_db\":false,\"mode\":\"passive\",\"mode.alarm\":3600,\"mode.timeout\":300,\"pruning\":\"fast\",\"tracing\":false}"
	
	func menuAppPath() -> String {
		return Bundle.main.executablePath!
	}

	func metaversePath() -> String {
		return Bundle.main.bundlePath + "/Contents/MacOS/mvsd"
	}

	func isAlreadyRunning() -> Bool {
		return NSRunningApplication.runningApplications(withBundleIdentifier: Bundle.main.bundleIdentifier!).count > 1

	}
	
	func isMetaverseRunning() -> Bool {
		if let pid = self.metaversePid {
			return kill(pid, 0) == 0
		}
		return false
	}
	
	func killMetaverse() {
		if let pid = self.metaversePid {
			kill(pid, SIGKILL)
		}
	}
	
	func openUI() {
		let metaverse = Process()
		metaverse.launchPath = "/usr/bin/open"
		metaverse.arguments = ["http:127.0.0.1:8820"]
		metaverse.launch()
	}
    
    func checkConn() -> String {
        let outPipe = Pipe()
        let errPipe = Pipe()
        let mvs = Process()
        mvs.standardOutput = outPipe
        mvs.standardError = errPipe
        mvs.launchPath = "/usr/sbin/lsof"
        mvs.arguments = ["-i:8820"]
        mvs.launch()
        let outData = outPipe.fileHandleForReading.availableData
        let outputString = String(data: outData, encoding: String.Encoding.utf8) ?? ""
        return outputString
    }
	
	func autostartEnabled() -> Bool {
		return itemReferencesInLoginItems().existingReference != nil
	}

	func itemReferencesInLoginItems() -> (existingReference: LSSharedFileListItem?, lastReference: LSSharedFileListItem?) {
		let itemUrl: UnsafeMutablePointer<Unmanaged<CFURL>?> = UnsafeMutablePointer<Unmanaged<CFURL>?>.allocate(capacity: 1)
		if let appUrl: NSURL = NSURL.fileURL(withPath: Bundle.main.bundlePath) as NSURL? {
			let loginItemsRef = LSSharedFileListCreate(
				nil,
				kLSSharedFileListSessionLoginItems.takeRetainedValue(),
				nil
				).takeRetainedValue() as LSSharedFileList?
			if loginItemsRef != nil {
				let loginItems: NSArray = LSSharedFileListCopySnapshot(loginItemsRef, nil).takeRetainedValue() as NSArray
				if(loginItems.count > 0)
				{
					let lastItemRef: LSSharedFileListItem = loginItems.lastObject as! LSSharedFileListItem
					for i in 0 ..< loginItems.count {
						let currentItemRef: LSSharedFileListItem = loginItems.object(at: i) as! LSSharedFileListItem
						if LSSharedFileListItemResolve(currentItemRef, 0, itemUrl, nil) == noErr {
							if let urlRef: NSURL =  itemUrl.pointee?.takeRetainedValue() {
								if urlRef.isEqual(appUrl) {
									return (currentItemRef, lastItemRef)
								}
							}
						}
					}
					//The application was not found in the startup list
					return (nil, lastItemRef)
				}
				else
				{
					let addAtStart: LSSharedFileListItem = kLSSharedFileListItemBeforeFirst.takeRetainedValue()
					return(nil, addAtStart)
				}
			}
		}
		return (nil, nil)
	}
	
	func toggleLaunchAtStartup() {
		let itemReferences = itemReferencesInLoginItems()
		let shouldBeToggled = (itemReferences.existingReference == nil)
		let loginItemsRef = LSSharedFileListCreate(
			nil,
			kLSSharedFileListSessionLoginItems.takeRetainedValue(),
			nil
			).takeRetainedValue() as LSSharedFileList?
		if loginItemsRef != nil {
			if shouldBeToggled {
				if let appUrl : CFURL = NSURL.fileURL(withPath: Bundle.main.bundlePath) as CFURL? {
					LSSharedFileListInsertItemURL(
						loginItemsRef,
						itemReferences.lastReference,
						nil,
						nil,
						appUrl,
						nil,
						nil
					)
				}
			} else {
				if let itemRef = itemReferences.existingReference {
					LSSharedFileListItemRemove(loginItemsRef,itemRef)
				}
			}
		}
	}

	func launchMetaverse() {
		self.commandLine = CommandLine.arguments.dropFirst().filter({ $0 != "ui"})
		
		let processes = GetBSDProcessList()!
		let metaverseProcess = processes.index(where: {
			var name = $0.kp_proc.p_comm
			let str = withUnsafePointer(to: &name) {
				$0.withMemoryRebound(to: UInt8.self, capacity: MemoryLayout.size(ofValue: name)) {
				String(cString: $0)
				}
			}
			return str == "metaverse"
		})
		
		if metaverseProcess == nil {
			let metaverse = Process()
			let p = self.metaversePath()
			metaverse.launchPath = p//self.metaversePath()
			metaverse.arguments = self.commandLine
			metaverse.launch()
			self.metaversePid = metaverse.processIdentifier

            var flag = false
            while (!flag) {
                if (self.checkConn() != "") {
                    flag = true
                }
            }
            self.openUI()
		} else {
			self.metaversePid = processes[metaverseProcess!].kp_proc.p_pid
		}
	}
	
	func applicationDidFinishLaunching(_ aNotification: Notification) {
		if self.isAlreadyRunning() {
			openUI()
			NSApplication.shared().terminate(self)
			return
		}

		self.launchMetaverse()
		Timer.scheduledTimer(withTimeInterval: 1.0, repeats: true, block: {_ in 
			if !self.isMetaverseRunning() {
				NSApplication.shared().terminate(self)
			}
		})
		
		let icon = NSImage(named: "statusIcon")
		icon?.isTemplate = true // best for dark mode
		statusItem.image = icon
		statusItem.menu = statusMenu
	}

	override func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
		if menuItem == self.startAtLogonMenuItem! {
			menuItem.state = self.autostartEnabled() ? NSOnState : NSOffState
		}
		return true
	}
	
	@IBAction func quitClicked(_ sender: NSMenuItem) {
		self.killMetaverse()
		NSApplication.shared().terminate(self)
	}
	
	@IBAction func openClicked(_ sender: NSMenuItem) {
		self.openUI()
	}
	
	@IBAction func startAtLogonClicked(_ sender: NSMenuItem) {
		self.toggleLaunchAtStartup()
	}

}

