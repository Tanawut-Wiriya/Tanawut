function isValidIPAddress(ip) {
    const octets = ip.split('.');
    if (octets.length !== 4) {
        return false;
    }
    for (let i = 0; i < octets.length; i++) {
        const octet = parseInt(octets[i], 10);
        if (isNaN(octet) || octet < 0 || octet > 255) {
            return false;
        }
    }
    return true;
}

document.addEventListener('DOMContentLoaded', function () {
    document.getElementById('networksConfig').addEventListener('submit', function (event) {
        event.preventDefault(); // Prevent default form submission

        const IPAddress = document.getElementById('IPAddress').value;
        const gateway = document.getElementById('gateway').value;
        const subnet = document.getElementById('subnet').value;
        const dns = document.getElementById('dns').value;

        // Validate IP Address
        if (!isValidIPAddress(IPAddress)) {
            alert('Invalid IP Address. Please enter a valid IP Address in the format x.x.x.x where x is between 0 and 255.');
            return;
        }

        if (!isValidIPAddress(gateway)) {
            alert('Invalid Gateway Address. Please enter a valid Gateway Address in the format x.x.x.x where x is between 0 and 255.');
            return;
        }

        if (!isValidIPAddress(subnet)) {
            alert('Invalid Subnet Mask. Please enter a valid Subnet Mask in the format x.x.x.x where x is between 0 and 255.');
            return;
        }

        if (!isValidIPAddress(dns)) {
            alert('Invalid DNS Address. Please enter a valid DNS Address in the format x.x.x.x where x is between 0 and 255.');
            return;
        }

        let body = `IPAddress=${IPAddress}&gateway=${gateway}&subnet=${subnet}&dns=${dns}`;

        fetch('/networksConfig', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded',
            },
            body: body
        })
            .then(response => {
                if (response.ok) {
                    window.location.reload();
                } else {
                    console.error('Failed to set mode');
                }
            })
            .catch(error => {
                console.error('Error:', error);
            });
    });

    document.getElementById("completeButton").addEventListener("click", function () {
        document.getElementById("actionInput").value = "complete";
        document.getElementById("saveForm").submit();
    });

    document.getElementById("restartButton").addEventListener("click", function () {
        document.getElementById("actionInput").value = "restart";
        document.getElementById("saveForm").submit();
    });

    function sendConfigAction(action) {
        fetch(`/saveConfig?action=${action}`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded',
            },
        })
            .then(response => {
                if (response.ok) {
                    console.log(`${action} action sent successfully`);
                    if (action === 'restart') {
                        alert('Restarting the device...');
                    } else {
                        alert('Configuration saved successfully');
                    }
                } else {
                    console.error(`Failed to send ${action} action`);
                }
            })
            .catch(error => {
                console.error('Error:', error);
            });
    }
});