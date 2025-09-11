// 工具函数
function checkLoginStatus() {
    const token = localStorage.getItem('auth_token');
    const authLinks = document.getElementById('auth-links');
    const userLinks = document.getElementById('user-links');

    if (token) {
        if (authLinks && userLinks) {
            authLinks.style.display = 'none';
            userLinks.style.display = 'block';
        }
        return true;
    }

    if (authLinks && userLinks) {
        authLinks.style.display = 'block';
        userLinks.style.display = 'none';
    }
    return false;
}

async function logout() {
    console.log('Logging out...');

    // 调用退出接口（如果有）
    try {
        const token = localStorage.getItem('auth_token');
        if (token) {
            await fetch('/api/logout', {
                method: 'POST',
                headers: {
                    'Authorization': `Bearer ${token}`
                }
            });
        }
    } catch (error) {
        console.error('Logout API error:', error);
    }

    // 清除本地存储
    localStorage.removeItem('auth_token');
    localStorage.removeItem('user_id');

    // 跳转到首页
    window.location.href = 'index.html';
}

// 增强的 API 调用函数
async function apiCall(url, options = {}) {
    const token = localStorage.getItem('auth_token');

    // 设置默认头
    const headers = {
        'Content-Type': 'application/json',
        ...options.headers
    };

    // 添加 Authorization 头（如果存在 token 且不是登录/注册请求）
    if (token && !url.includes('/api/login') && !url.includes('/api/register')) {
        headers['Authorization'] = `Bearer ${token}`;
    }

    try {
        const response = await fetch(url, {
            headers,
            ...options
        });

        if (response.status === 401) {
            // Token 过期或无效
            console.log('Token expired or invalid, redirecting to login');
            logout();
            throw new Error('Authentication failed');
        }

        const data = await response.json();

        if (!response.ok) {
            throw new Error(data.message || `HTTP error! status: ${response.status}`);
        }

        return data;
    } catch (error) {
        console.error('API call failed:', error);
        throw error;
    }
}

// 表单验证函数
function validateEmail(email) {
    const re = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
    return re.test(email);
}

function validatePassword(password) {
    return password.length >= 6;
}

function validateUsername(username) {
    return username.length >= 3 && username.length <= 20;
}