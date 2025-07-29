use qrvmc_sys as ffi;

/// QRVMC address
pub type Address = ffi::qrvmc_address;

/// QRVMC 32 bytes value (used for hashes)
pub type Bytes32 = ffi::qrvmc_bytes32;

/// QRVMC big-endian 256-bit integer
pub type Uint256 = ffi::qrvmc_uint256be;

/// QRVMC call kind.
pub type MessageKind = ffi::qrvmc_call_kind;

/// QRVMC message (call) flags.
pub type MessageFlags = ffi::qrvmc_flags;

/// QRVMC status code.
pub type StatusCode = ffi::qrvmc_status_code;

/// QRVMC access status.
pub type AccessStatus = ffi::qrvmc_access_status;

/// QRVMC storage status.
pub type StorageStatus = ffi::qrvmc_storage_status;

/// QRVMC VM revision.
pub type Revision = ffi::qrvmc_revision;

#[cfg(test)]
mod tests {
    use super::*;

    // These tests check for Default, PartialEq and Clone traits.
    #[test]
    fn address_smoke_test() {
        let a = ffi::qrvmc_address::default();
        let b = Address::default();
        assert_eq!(a.clone(), b.clone());
    }

    #[test]
    fn bytes32_smoke_test() {
        let a = ffi::qrvmc_bytes32::default();
        let b = Bytes32::default();
        assert_eq!(a.clone(), b.clone());
    }

    #[test]
    fn uint26be_smoke_test() {
        let a = ffi::qrvmc_uint256be::default();
        let b = Uint256::default();
        assert_eq!(a.clone(), b.clone());
    }

    #[test]
    fn message_kind() {
        assert_eq!(MessageKind::QRVMC_CALL, ffi::qrvmc_call_kind::QRVMC_CALL);
        assert_eq!(
            MessageKind::QRVMC_DELEGATECALL,
            ffi::qrvmc_call_kind::QRVMC_DELEGATECALL
        );
        assert_eq!(MessageKind::QRVMC_CREATE, ffi::qrvmc_call_kind::QRVMC_CREATE);
    }

    #[test]
    fn message_flags() {
        assert_eq!(MessageFlags::QRVMC_STATIC, ffi::qrvmc_flags::QRVMC_STATIC);
    }

    #[test]
    fn status_code() {
        assert_eq!(
            StatusCode::QRVMC_SUCCESS,
            ffi::qrvmc_status_code::QRVMC_SUCCESS
        );
        assert_eq!(
            StatusCode::QRVMC_FAILURE,
            ffi::qrvmc_status_code::QRVMC_FAILURE
        );
    }

    #[test]
    fn access_status() {
        assert_eq!(
            AccessStatus::QRVMC_ACCESS_COLD,
            ffi::qrvmc_access_status::QRVMC_ACCESS_COLD
        );
        assert_eq!(
            AccessStatus::QRVMC_ACCESS_WARM,
            ffi::qrvmc_access_status::QRVMC_ACCESS_WARM
        );
    }

    #[test]
    fn storage_status() {
        assert_eq!(
            StorageStatus::QRVMC_STORAGE_ASSIGNED,
            ffi::qrvmc_storage_status::QRVMC_STORAGE_ASSIGNED
        );
        assert_eq!(
            StorageStatus::QRVMC_STORAGE_MODIFIED,
            ffi::qrvmc_storage_status::QRVMC_STORAGE_MODIFIED
        );
    }

    #[test]
    fn revision() {
        assert_eq!(Revision::QRVMC_SHANGHAI, ffi::qrvmc_revision::QRVMC_SHANGHAI);
    }
}
