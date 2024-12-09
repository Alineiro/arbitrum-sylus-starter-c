import { createPublicClient, createWalletClient, http, parseAbi } from "viem"
import { arbitrumSepolia } from "viem/chains"
import { privateKeyToAccount } from "viem/accounts"
import { stringToBytes } from "viem/utils"; // Import utility to convert string to bytes
import "dotenv/config"

const ABI = parseAbi([
	"function register_developer() public returns (string)",
	"function submit_app_hash(address,bytes32) public returns (string)",
	"function verify_app_hash(bytes32) public returns (bytes32)",
])

const account = privateKeyToAccount((process as any).env.PRIVATE_KEY)

const client = createWalletClient({
  chain: arbitrumSepolia,
  transport: http(),
  account,
})

const publicClient = createPublicClient({
  chain: arbitrumSepolia,
  transport: http(),
})

// https://sepolia.arbiscan.io/address/const CONTRACT_ADDRESS = "0x46be8751225be83d7a9b97fec0214c53795d8477"
const CONTRACT_ADDRESS = "0x29fd053e5c0b4d8829a8420b8c95ef9ac055084b"

async function register() {
    const result = await client.writeContract({
        abi: ABI,
        address: CONTRACT_ADDRESS,
        functionName: "register_developer",
    });

    console.debug(`Developer registered: ${result}`);
    return result;
}

async function submit() {
    const developerAddress = "0xaB55FD34340b5e8fEE8615690431649046fE135b";
    const appHash = "0xabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdebadbb";
    //Same appHash cannot be submitted multiple times

    const nonce = await publicClient.getTransactionCount({ address: developerAddress });
    const result = await client.writeContract({
        abi: ABI,
        address: CONTRACT_ADDRESS,
        functionName: "submit_app_hash",
        args: [developerAddress, appHash],
	nonce: nonce + 1,	//Weird bug, current nonce isn't synced, it's always 1 less, so +1 fixes it
    });

    console.debug(`App hash submitted: ${result}`);
    return result;
}

async function verify(newAppHash) {

    const result = await publicClient.readContract({
        abi: ABI,
        address: CONTRACT_ADDRESS,
        functionName: "verify_app_hash",
        args: [newAppHash],
    });

    console.debug(`Verification result: ${result}`);
}

async function testContract() {
    const registeredDev = await register(); // Register the developer
    const newAppHash = await submit(); // Submit the app hash
    await verify(newAppHash); // Verify the app hash
}

testContract().catch(console.error);

