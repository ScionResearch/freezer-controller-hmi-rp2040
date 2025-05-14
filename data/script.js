// Navigation
document.querySelectorAll('nav a').forEach(link => {
    link.addEventListener('click', (e) => {
        e.preventDefault();
        const page = e.target.dataset.page;
        document.querySelectorAll('.page').forEach(p => p.classList.remove('active'));
        document.querySelector(`#${page}`).classList.add('active');
        document.querySelectorAll('nav a').forEach(a => a.classList.remove('active'));
        e.target.classList.add('active');
    });
});

// Chart configuration and data
let sensorChart = null;
const chartData = {
    labels: [],
    temperature: [],
    humidity: [],
    setpoint: []
};

// Maximum number of data points to show on chart
const MAX_DATA_POINTS = 21600;

// Initialize dashboard chart
function initSensorChart() {
    const ctx = document.getElementById('sensorChart').getContext('2d');
    
    // Create the chart with dual y-axes (temperature and humidity)
    sensorChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                {
                    label: 'Temperature (°C)',
                    data: [],
                    borderColor: 'rgb(255, 99, 132)',
                    backgroundColor: 'rgba(255, 99, 132, 0.1)',
                    borderWidth: 1.0,
                    tension: 0.2,
                    yAxisID: 'y',
                    pointRadius: 1.0,
                    pointHoverRadius: 3
                },
                {
                    label: 'Humidity (%)',
                    data: [],
                    borderColor: 'rgb(54, 162, 235)',
                    backgroundColor: 'rgba(54, 162, 235, 0.1)',
                    borderWidth: 1.0,
                    tension: 0.2,
                    yAxisID: 'y1',
                    pointRadius: 1.0,
                    pointHoverRadius: 3
                },
                {
                    label: 'Setpoint (°C)',
                    data: [],
                    borderColor: 'rgb(255, 159, 64)',
                    backgroundColor: 'rgba(255, 159, 64, 0.1)',
                    borderWidth: 1.5,
                    borderDash: [5, 3],
                    tension: 0,
                    yAxisID: 'y',
                    pointRadius: 0,
                    pointHoverRadius: 0,
                    fill: false
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            interaction: {
                mode: 'index',
                intersect: false,
            },
            scales: {
                x: {
                    grid: {
                        display: false
                    },
                    ticks: {
                        maxTicksLimit: 10,
                        autoSkip: true
                    }
                },
                y: {
                    type: 'linear',
                    display: true,
                    position: 'left',
                    title: {
                        display: true,
                        text: 'Temperature (°C)'
                    },
                    beginAtZero: false,
                    grace: '5%',
                    ticks: {
                        precision: 1
                    },
                    adapters: {
                      autoRanging: true
                    }
                },
                y1: {
                    type: 'linear',
                    display: true,
                    position: 'right',
                    title: {
                        display: true,
                        text: 'Humidity (%)'
                    },
                    beginAtZero: false,
                    grace: '5%',
                    ticks: {
                        precision: 1
                    },
                    adapters: {
                      autoRanging: true
                    },
                    grid: {
                        drawOnChartArea: false
                    }
                }
            },
            animation: {
                duration: 500
            },
            plugins: {
                legend: {
                    labels: {
                        boxWidth: 12,
                        font: {
                            size: 11
                        }
                    }
                }
            }
        }
    });
}

// Sensor data fetching and dashboard updates
async function fetchSensorData() {
    try {
        const response = await fetch('/api/sensor');
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        return await response.json();
    } catch (error) {
        console.error('Error fetching sensor data:', error);
        return null;
    }
}

function updateSensorStatus(status) {
    const statusIndicator = document.querySelector('.sensor-status-indicator');
    const statusText = document.getElementById('sensorStatusText');
    
    // Remove all previous status classes
    statusIndicator.classList.remove('status-online', 'status-offline', 'status-locked');
    
    // Set the appropriate status
    if (status === 'online') {
        statusIndicator.classList.add('status-online');
        statusText.textContent = 'Online';
    } else if (status === 'offline') {
        statusIndicator.classList.add('status-offline');
        statusText.textContent = 'Offline';
    } else if (status === 'locked') {
        statusIndicator.classList.add('status-locked');
        statusText.textContent = 'Locked';
    } else {
        statusIndicator.classList.add('status-offline');
        statusText.textContent = 'Unknown';
    }
}

function formatDecimal(value, precision = 2) {
    return value !== undefined && !isNaN(value) 
        ? parseFloat(value).toFixed(precision) 
        : '--';
}

function updateSensorDashboard(data) {
    if (!data) return;
    
    // Update sensor status
    updateSensorStatus(data.status);
    
    // Update sensor IDs and info
    if (data.tempSensorID_1 !== undefined) {
        document.getElementById('tempSensor1ID').textContent = data.tempSensorID_1;
    }
    if (data.tempSensorID_2 !== undefined) {
        document.getElementById('tempSensor2ID').textContent = data.tempSensorID_2;
    }
    
    // Update sensor readings
    if (data.status === 'online' || data.status === 'offline') {
        // Temperature
        document.getElementById('temperatureValue').textContent = `${formatDecimal(data.temperature)} °C`;
        
        // Temperature setpoint
        if (data.setpoint !== undefined) {
            document.getElementById('tempSetpoint').textContent = `${formatDecimal(data.setpoint)} °C`;
        }
        
        // Compressor status
        updateCompressorStatus(data.compressorRunning);
        
        // Humidity
        document.getElementById('humidityValue').textContent = `${formatDecimal(data.humidity)} %`;
        
        // Dew Point
        document.getElementById('dewPointValue').textContent = `${formatDecimal(data.dewPoint)} °C`;
        
        // Pressure
        document.getElementById('pressureValue').textContent = `${formatDecimal(data.pressure)} hPa`;
        
        // Delta T
        document.getElementById('deltaTValue').textContent = `${formatDecimal(data.deltaT)} °C`;
        
        // Delta H
        document.getElementById('deltaHValue').textContent = `${formatDecimal(data.deltaH)} %`;
        
        // Update chart data
        updateChartData(data);
    }
}

function updateCompressorStatus(isRunning) {
    const statusElement = document.getElementById('compressorStatus');
    const textElement = statusElement.querySelector('.compressor-text');
    
    if (isRunning) {
        statusElement.classList.add('running');
        textElement.textContent = 'Compressor running';
    } else {
        statusElement.classList.remove('running');
        textElement.textContent = 'Compressor idle';
    }
}

function updateChartData(data) {
    if (!sensorChart) return;
    
    // Only add data if the sensor is online
    if (data.status === 'online') {
        // Get current time for label
        const now = new Date();
        const timeString = now.toLocaleTimeString('en-US', { 
            hour: '2-digit', 
            minute: '2-digit',
            second: '2-digit',
            hour12: false 
        });
        
        // Add new data point
        chartData.labels.push(timeString);
        chartData.temperature.push(data.temperature);
        chartData.humidity.push(data.humidity);
        
        // Add setpoint value (constant for the whole chart)
        if (data.setpoint !== undefined) {
            chartData.setpoint.push(data.setpoint);
        } else {
            // If no setpoint data, add null to maintain array length
            chartData.setpoint.push(null);
        }
        
        // Limit data points
        if (chartData.labels.length > MAX_DATA_POINTS) {
            chartData.labels.shift();
            chartData.temperature.shift();
            chartData.humidity.shift();
            chartData.setpoint.shift();
        }
        
        // Update chart with new data
        sensorChart.data.labels = chartData.labels;
        sensorChart.data.datasets[0].data = chartData.temperature;
        sensorChart.data.datasets[1].data = chartData.humidity;
        sensorChart.data.datasets[2].data = chartData.setpoint;
        
        // Calculate min/max for auto-ranging (if we have at least 2 data points)
        if (chartData.temperature.length >= 2) {
            const tempValues = [...chartData.temperature].filter(val => !isNaN(val));
            const humidValues = [...chartData.humidity].filter(val => !isNaN(val));
            
            // Include setpoint in temperature range calculation
            if (data.setpoint !== undefined) {
                tempValues.push(data.setpoint);
            }
            
            if (tempValues.length > 0) {
                const tempMin = Math.min(...tempValues);
                const tempMax = Math.max(...tempValues);
                const tempRange = tempMax - tempMin;
                
                // Set temperature axis range with some padding
                sensorChart.options.scales.y.min = Math.max(-30, tempMin - (tempRange * 0.1));
                sensorChart.options.scales.y.max = Math.min(50, tempMax + (tempRange * 0.1));
            }
            
            if (humidValues.length > 0) {
                const humidMin = Math.min(...humidValues);
                const humidMax = Math.max(...humidValues);
                const humidRange = humidMax - humidMin;
                
                // Set humidity axis range with some padding
                sensorChart.options.scales.y1.min = Math.max(0, humidMin - (humidRange * 0.1));
                sensorChart.options.scales.y1.max = Math.min(100, humidMax + (humidRange * 0.1));
            }
        }
        
        sensorChart.update('none'); // Use 'none' to disable animations when updating
    }
}

// Main function to update sensor dashboard
async function updateDashboard() {
    const data = await fetchSensorData();
    if (data) {
        updateSensorDashboard(data);
    }
}

// Network and sensor data updates
async function updateNetworkInfo() {
    try {
        const response = await fetch('/api/network');
        const settings = await response.json();
        const currentIPElement = document.getElementById('currentIP');
        const macAddressElement = document.getElementById('macAddress');
        
        if (currentIPElement) {
            currentIPElement.textContent = settings.ip || 'Not Connected';
        }
        if (macAddressElement) {
            macAddressElement.textContent = settings.mac || '';
        }
    } catch (error) {
        console.error('Error updating network info:', error);
    }
}

async function saveTimeSettings() {
    const date = document.getElementById('currentDate').value;
    const fullTime = document.getElementById('currentTime').value;
    // Extract only hours and minutes from the time value
    const timeParts = fullTime.split(':');
    const time = `${timeParts[0]}:${timeParts[1]}`; // HH:MM format
    const timezone = document.getElementById('timezone').value;
    const ntpEnabled = document.getElementById('enableNTP').checked;
    const dstEnabled = document.getElementById('enableDST').checked;

    // Show loading toast
    const loadingToast = showToast('info', 'Saving...', 'Updating date and time settings', 10000);

    try {
        const response = await fetch('/api/time', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({
                date,
                time,
                timezone,
                ntpEnabled,
                dstEnabled
            }),
        });

        // Remove loading toast
        if (loadingToast.parentNode) {
            loadingToast.parentNode.removeChild(loadingToast);
        }

        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const result = await response.json();
        console.log('Time settings updated:', result);
        
        // Show success toast
        showToast('success', 'Success', 'Date and time settings saved successfully');
        
        // Update the time display immediately
        updateLiveClock();
    } catch (error) {
        console.error('Error saving time settings:', error);
        
        // Show error toast
        showToast('error', 'Error', 'Failed to save date and time settings');
    }
}

// Update live clock display
async function updateLiveClock() {
    const clockElement = document.getElementById('liveClock');
    if (clockElement) {
        try {
            const response = await fetch('/api/time');
            const data = await response.json();
            console.log('Time data received:', data);

            if (data.date && data.time && data.timezone) {
                const isoDateTime = `${data.date}T${data.time}${data.timezone}`;

               const now = new Date(isoDateTime);

               const formattedTime = new Intl.DateTimeFormat('en-NZ', {
                    year: 'numeric',
                    month: 'long',
                    day: 'numeric',
                    hour: '2-digit',
                    minute: '2-digit',
                    second: '2-digit',
                    timeZone: data.timezone
                }).format(now) + ' ' + data.timezone + (data.dst ? ' (DST)' : '');
           
                clockElement.textContent = formattedTime;
            }

            // Update NTP status if NTP is enabled
            const ntpStatusContainer = document.getElementById('ntpStatusContainer');
            const ntpStatusElement = document.getElementById('ntpStatus');
            const lastNtpUpdateElement = document.getElementById('lastNtpUpdate');
            
            if (data.ntpEnabled) {
                ntpStatusContainer.style.display = 'block';
                
                if (ntpStatusElement && data.hasOwnProperty('ntpStatus')) {
                    // Clear previous classes
                    ntpStatusElement.className = 'ntp-status';
                    
                    // Add status text and class
                    let statusText = '';
                    switch (data.ntpStatus) {
                        case 0: // NTP_STATUS_CURRENT
                            statusText = 'Current';
                            ntpStatusElement.classList.add('ntp-status-current');
                            break;
                        case 1: // NTP_STATUS_STALE
                            statusText = 'Stale';
                            ntpStatusElement.classList.add('ntp-status-stale');
                            break;
                        case 2: // NTP_STATUS_FAILED
                        default:
                            statusText = 'Failed';
                            ntpStatusElement.classList.add('ntp-status-failed');
                            break;
                    }
                    ntpStatusElement.textContent = statusText;
                }
                
                if (lastNtpUpdateElement && data.hasOwnProperty('lastNtpUpdate')) {
                    lastNtpUpdateElement.textContent = data.lastNtpUpdate;
                }
            } else {
                ntpStatusContainer.style.display = 'none';
            }
        } catch (error) {
            console.error('Error updating clock:', error);
        }
    }
}

// Update date/time input fields based on NTP state
function updateInputStates() {
    const ntpEnabled = document.getElementById('enableNTP').checked;
    const dateInput = document.getElementById('currentDate');
    const timeInput = document.getElementById('currentTime');
    
    if (dateInput && timeInput) {
        dateInput.disabled = ntpEnabled;
        timeInput.disabled = ntpEnabled;
    }
}

// Function to load initial settings
async function loadInitialSettings() {
    try {
        const response = await fetch('/api/time');
        const data = await response.json();
        
        if (data) {
            document.getElementById('enableNTP').checked = data.ntpEnabled;
            document.getElementById('enableDST').checked = data.dst;
            document.getElementById('timezone').value = data.timezone;
            updateInputStates();
        }
    } catch (error) {
        console.error('Error loading initial settings:', error);
    }
}

// Event listeners
document.addEventListener('DOMContentLoaded', () => {
    // Initialize dashboard
    initSensorChart();
    updateDashboard();
    
    // Set up interval for dashboard updates (every 2 seconds)
    setInterval(updateDashboard, 2000);
    
    // Load initial settings
    loadInitialSettings();  // Load initial NTP and timezone settings
    loadNetworkSettings();  // Load initial network settings
    loadControllerSettings(); // Load initial controller settings
    
    // Set up time display updates
    updateLiveClock();
    setInterval(updateLiveClock, 1000);
    
    // Save time settings button
    const saveTimeBtn = document.getElementById('saveTimeBtn');
    if (saveTimeBtn) {
        saveTimeBtn.addEventListener('click', saveTimeSettings);
    }
    
    // NTP toggle
    const enableNTP = document.getElementById('enableNTP');
    if (enableNTP) {
        enableNTP.addEventListener('change', updateInputStates);
    }
    
    // Static IP toggle
    const ipConfig = document.getElementById('ipConfig');
    if (ipConfig) {
        ipConfig.addEventListener('change', updateStaticFields);
        // Set initial state
        updateStaticFields();
    }
    
    // Controller settings form
    const controllerForm = document.getElementById('controllerForm');
    if (controllerForm) {
        controllerForm.addEventListener('submit', saveControllerSettings);
    }
});

function updateStaticFields() {
    const ipConfig = document.getElementById('ipConfig');
    const staticSettings = document.getElementById('staticSettings');
    
    if (ipConfig.value === 'static') {
        staticSettings.style.display = 'block';
    } else {
        staticSettings.style.display = 'none';
    }
}

// Controller settings handling
async function loadControllerSettings() {
    try {
        const response = await fetch('/api/control');
        const data = await response.json();
        
        document.getElementById('temperatureSetpoint').value = data.temperature_setpoint.toFixed(1) || 0;
        document.getElementById('compressorOnHysteresis').value = data.compressor_on_hysteresis.toFixed(2) || 0;
        document.getElementById('compressorOffHysteresis').value = data.compressor_off_hysteresis.toFixed(2) || 0;
        document.getElementById('fanSpeed').value = data.fan_speed || 0;
        document.getElementById('modbusTcpPort').value = data.modbus_tcp_port || 502;
    } catch (error) {
        console.error('Error loading controller settings:', error);
    }
}

async function saveControllerSettings(e) {
    if (e) e.preventDefault();
    
    // Get form values
    const temperatureSetpoint = parseFloat(document.getElementById('temperatureSetpoint').value);
    const compressorOnHysteresis = parseFloat(document.getElementById('compressorOnHysteresis').value);
    const compressorOffHysteresis = parseFloat(document.getElementById('compressorOffHysteresis').value);
    const fanSpeed = parseInt(document.getElementById('fanSpeed').value, 10);
    const modbusTcpPort = parseInt(document.getElementById('modbusTcpPort').value, 10);

    // Validate temperature setpoint
    if (isNaN(temperatureSetpoint) || temperatureSetpoint < -20 || temperatureSetpoint > 5) {
        showToast('error', 'Validation Error', 'Invalid temperature setpoint. Must be between -20 and 5.');
        return;
    }
    
    // Validate compressor on hysteresis
    if (isNaN(compressorOnHysteresis) || compressorOnHysteresis < -4 || compressorOnHysteresis > 4) {
        showToast('error', 'Validation Error', 'Invalid compressor on hysteresis. Must be between -4 and 4.');
        return;
    }
    
    // Validate compressor off hysteresis
    if (isNaN(compressorOffHysteresis) || compressorOffHysteresis < -4 || compressorOffHysteresis > 4) {
        showToast('error', 'Validation Error', 'Invalid compressor off hysteresis. Must be between -4 and 4.');
        return;
    }

    // Validate fan speed
    if (isNaN(fanSpeed) || fanSpeed < 0 || fanSpeed > 100) {
        showToast('error', 'Validation Error', 'Invalid fan speed. Must be between 0 and 100.');
        return;
    }
    
    // Validate port number
    if (isNaN(modbusTcpPort) || modbusTcpPort < 1 || modbusTcpPort > 65535) {
        showToast('error', 'Validation Error', 'Invalid Modbus TCP port. Must be between 1 and 65535.');
        return;
    }

    console.log('Saving controller settings: ' + JSON.stringify({
        temperature_setpoint: temperatureSetpoint,
        compressor_on_hysteresis: compressorOnHysteresis,
        compressor_off_hysteresis: compressorOffHysteresis,
        fan_speed: fanSpeed,
        modbus_tcp_port: modbusTcpPort
    }));
    
    try {
        const response = await fetch('/api/control', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({
                temperature_setpoint: temperatureSetpoint,
                compressor_on_hysteresis: compressorOnHysteresis,
                compressor_off_hysteresis: compressorOffHysteresis,
                fan_speed: fanSpeed,
                modbus_tcp_port: modbusTcpPort
            })
        });
        
        if (!response.ok) {
            const errorData = await response.json();
            throw new Error(errorData.error || `HTTP error! status: ${response.status}`);
        }
        
        const result = await response.json();
        console.log('Controller settings updated:', result);
        
        showToast('success', 'Success', 'Controller settings saved successfully.');
        
    } catch (error) {
        console.error('Error saving controller settings:', error);
        showToast('error', 'Error', 'Failed to save controller settings: ' + error.message);
    }
}

// Toast notification functions
function showToast(type, title, message, duration = 3000) {
    const toastContainer = document.getElementById('toastContainer');
    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;
    
    toast.innerHTML = `
        <div class="toast-header">
            <strong>${title}</strong>
            <button type="button" class="toast-close">&times;</button>
        </div>
        <div class="toast-body">
            ${message}
        </div>
    `;
    
    toastContainer.appendChild(toast);
    
    // Add event listener to close button
    const closeButton = toast.querySelector('.toast-close');
    closeButton.addEventListener('click', () => {
        if (toast.parentNode) {
            toast.parentNode.removeChild(toast);
        }
    });
    
    // Remove toast after duration
    if (duration > 0) {
        setTimeout(() => {
            if (toast.parentNode) {
                toast.parentNode.removeChild(toast);
            }
        }, duration);
    }
    
    return toast;
}

// Network settings handling
document.getElementById('networkForm').addEventListener('submit', async function(e) {
    e.preventDefault();
    const statusDiv = document.getElementById('networkStatus');
    
    // Show loading message
    statusDiv.textContent = 'Saving network settings...';
    statusDiv.className = 'status info';
    
    // Get form values
    const mode = document.getElementById('ipConfig').value;
    let data = {
        mode: mode,
        hostname: document.getElementById('hostName').value,
        ntp: document.getElementById('ntpServer').value
    };
    
    // Add static IP configuration if selected
    if (mode === 'static') {
        data.ip = document.getElementById('ipAddress').value;
        data.subnet = document.getElementById('subnetMask').value;
        data.gateway = document.getElementById('gateway').value;
        data.dns = document.getElementById('dns').value;
        
        // Validate IP addresses
        const ipPattern = /^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
        if (!ipPattern.test(data.ip)) {
            statusDiv.textContent = 'Invalid IP address format';
            statusDiv.className = 'status error';
            return;
        }
        if (!ipPattern.test(data.subnet)) {
            statusDiv.textContent = 'Invalid subnet mask format';
            statusDiv.className = 'status error';
            return;
        }
        if (!ipPattern.test(data.gateway)) {
            statusDiv.textContent = 'Invalid gateway format';
            statusDiv.className = 'status error';
            return;
        }
        if (!ipPattern.test(data.dns)) {
            statusDiv.textContent = 'Invalid DNS server format';
            statusDiv.className = 'status error';
            return;
        }
    }
    
    try {
        const response = await fetch('/api/network', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(data)
        });
        
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const result = await response.json();
        console.log('Network settings updated:', result);
        
        statusDiv.textContent = 'Network settings saved. System will reboot to apply changes...';
        statusDiv.className = 'status success';
        
        // Disable form inputs during reboot
        const inputs = document.querySelectorAll('#networkForm input, #networkForm select, #networkForm button');
        inputs.forEach(input => {
            input.disabled = true;
        });
        
        // Show countdown for reboot
        let countdown = 5;
        const countdownInterval = setInterval(() => {
            statusDiv.textContent = `Network settings saved. System will reboot in ${countdown} seconds...`;
            countdown--;
            
            if (countdown < 0) {
                clearInterval(countdownInterval);
                statusDiv.textContent = 'Waiting for system to come back online...';
                
                // Check for system availability
                checkSystemAvailability();
            }
        }, 1000);
        
    } catch (error) {
        console.error('Error saving network settings:', error);
        statusDiv.textContent = 'Failed to save network settings: ' + error.message;
        statusDiv.className = 'status error';
    }
});

// Load network settings
async function loadNetworkSettings() {
    try {
        const response = await fetch('/api/network');
        const data = await response.json();
        
        document.getElementById('ipConfig').value = data.mode;
        document.getElementById('hostName').value = data.hostname;
        document.getElementById('ntpServer').value = data.ntp;
        document.getElementById('currentIP').textContent = data.ip;
        document.getElementById('macAddress').textContent = data.mac;
        
        // Static IP fields
        if (data.mode === 'static') {
            document.getElementById('ipAddress').value = data.ip;
            document.getElementById('subnetMask').value = data.subnet;
            document.getElementById('gateway').value = data.gateway;
            document.getElementById('dns').value = data.dns;
            document.getElementById('staticSettings').style.display = 'block';
        } else {
            document.getElementById('staticSettings').style.display = 'none';
        }
    } catch (error) {
        console.error('Error loading network settings:', error);
    }
}

// System availability check
async function checkSystemAvailability() {
    // Try to connect to the server every 2 seconds
    const availabilityCheck = setInterval(async () => {
        try {
            const response = await fetch('/api/network', { timeout: 2000 });
            if (response.ok) {
                clearInterval(availabilityCheck);
                
                // System is back online
                const statusDiv = document.getElementById('networkStatus');
                statusDiv.textContent = 'System is back online.';
                
                // Re-enable form inputs
                const inputs = document.querySelectorAll('#networkForm input, #networkForm select, #networkForm button');
                inputs.forEach(input => {
                    input.disabled = false;
                });
                
                // Reload network settings
                loadNetworkSettings();
                
                // Fade out status message after a while
                setTimeout(() => {
                    statusDiv.style.opacity = '0';
                    setTimeout(() => {
                        statusDiv.textContent = '';
                        statusDiv.className = '';
                        statusDiv.style.opacity = '1';
                    }, 500);
                }, 3000);
            }
        } catch (error) {
            console.log('System still rebooting...');
        }
    }, 2000);
}

document.addEventListener('DOMContentLoaded', () => {
    const rebootButton = document.getElementById('rebootButton');
    const rebootModal = document.getElementById('rebootModal');
    const cancelReboot = document.getElementById('cancelReboot');
    const confirmReboot = document.getElementById('confirmReboot');
    
    console.log("Reboot elements:", { 
        rebootButton: !!rebootButton, 
        rebootModal: !!rebootModal, 
        cancelReboot: !!cancelReboot, 
        confirmReboot: !!confirmReboot 
    });
    
    // Show reboot confirmation modal
    if (rebootButton) {
        console.log("Adding click listener to reboot button");
        rebootButton.addEventListener('click', (e) => {
            console.log("Reboot button clicked");
            rebootModal.style.display = 'flex';
            e.preventDefault(); // Prevent any default actions
        });
    }
    
    // Cancel reboot
    if (cancelReboot) {
        cancelReboot.addEventListener('click', () => {
            rebootModal.style.display = 'none';
        });
    }
    
    // Confirm reboot
    if (confirmReboot) {
        confirmReboot.addEventListener('click', async () => {
            rebootModal.style.display = 'none';
            
            // Show loading toast
            const loadingToast = showToast('info', 'Rebooting...', 'System is rebooting, please wait', 10000);
            
            try {
                const response = await fetch('/api/system/reboot', {
                    method: 'POST'
                });
                
                // Remove loading toast
                if (loadingToast.parentNode) {
                    loadingToast.parentNode.removeChild(loadingToast);
                }
                
                if (!response.ok) {
                    throw new Error(`HTTP error! status: ${response.status}`);
                }
                
                // Show success toast
                showToast('success', 'Reboot Initiated', 'System is rebooting. This page will reload when the system is back online.');
                
                // Check for system availability after reboot
                setTimeout(() => {
                    checkSystemAvailability();
                }, 5000);
                
            } catch (error) {
                console.error('Error initiating reboot:', error);
                
                // Remove loading toast
                if (loadingToast.parentNode) {
                    loadingToast.parentNode.removeChild(loadingToast);
                }
                
                // Show error toast
                showToast('error', 'Reboot Failed', 'Failed to initiate system reboot');
            }
        });
    }
});