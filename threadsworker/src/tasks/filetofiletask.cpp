#include "filetofiletask.h"

#include "../system/interactivefactory.h"

namespace twPro {

    static size_t OPTIMAL_NOTIFIER_QUEUE_SIZE = 5;

    FileToFileTask::FileToFileTask(const twPro::SourceToResultTemplateTask_params & _params) noexcept :
        SourceToResultTemplateTask(_params)
    {
    }


    FileToFileTask::~FileToFileTask() noexcept
    {
    }

    Result<TASK_CODES> FileToFileTask::preCheck(const twPro::SourceToResultTemplateTask_params & _params) noexcept
    {
        if (_params.blockSize < 1) {
            return { TASK_CODES::ERROR, "Block size can't be less than 1" };
        }
        else if (_params.source.empty() || _params.result.empty()) {
            return { TASK_CODES::ERROR, "File path can't be empty" };
        }
        else if (_params.source == _params.result) {
            return { TASK_CODES::ERROR, "Input and output files can't have similar path" };
        }

        return TASK_CODES::OK;
    }

    Result<TASK_CODES> FileToFileTask::setupSources(std::shared_ptr<twPro::LRDataBuffer> _sourceBufferPtr, std::shared_ptr<twPro::LRDataBuffer> _resultBufferPtr) noexcept
    {
        // === SETUP RESOURCES  ===

        auto fileReader_RetVal = createFileReader(parameters().source);

        if (!fileReader_RetVal.result) {
            return fileReader_RetVal.result;
        }

        m_sourceFilePtr = fileReader_RetVal.value;
        m_sourceFilePtr->setProducerBuffer(_sourceBufferPtr);

        auto fileWriter_RetVal = createFileWriter(parameters().result);

        if (!fileWriter_RetVal.result) {
            return fileWriter_RetVal.result;
        }

        m_resultFilePtr = fileWriter_RetVal.value;
        m_resultFilePtr->setConsumerBuffer(_resultBufferPtr);

        // === PRINT MAIN INFO ===

        B_INFO << "Task block size: " << parameters().blockSize << " bytes" << E_INFO;
        B_INFO << "Source data length: " << totalData() << " bytes" << E_INFO;

        return TASK_CODES::OK;
    }

    std::function<void()> FileToFileTask::prepareSourceJob(std::atomic_bool & _stopFlag, Result<TASK_CODES> & _ret) noexcept
    {
        auto fileReaderJob = [this, &_stopFlag, &_ret]() {

            _ret = m_sourceFilePtr->work(_stopFlag);

            if (!_ret) {
                _stopFlag = true;
            }

        };

        return fileReaderJob;
    }

    std::function<void()> FileToFileTask::prepareResultJob(std::atomic_bool & _stopFlag, Result<TASK_CODES> & _ret) noexcept
    {
        auto fileWriterJob = [this, &_stopFlag, &_ret]() {

            _ret = m_resultFilePtr->work(_stopFlag);

            if (!_ret) {
                _stopFlag = true;
            }

        };

        return fileWriterJob;
    }

    std::function<void()> FileToFileTask::prepareWorkerJob(std::shared_ptr<twPro::IWorker> _worker, std::atomic_bool & _stopFlag, Result<TASK_CODES> & _ret) noexcept
    {
        auto workerJob = [_worker, &_stopFlag, &_ret]() {

            _ret = _worker->work(_stopFlag);

            if (!_ret) {
                _stopFlag = true;
            }

        };

        return workerJob;
    }

    void FileToFileTask::setupNotifiers(twPro::DataChannel & _channel, std::atomic_bool & _stopFlag, Result<TASK_CODES> & _ret) noexcept
    {
        auto writerNotifier = _channel.createNotifier<size_t>(OPTIMAL_NOTIFIER_QUEUE_SIZE);
        auto progress = interactive()->createProgressBar();

        m_resultFilePtr->setNotifier_currentConsumedDataUnits(writerNotifier);

        writerNotifier->setCallBack([this, &_stopFlag, progress](const size_t & _val) {

            progress->notify(IInteractive::Progress("Progress", _val, totalWriteDataUnits()));

            if (_val >= totalWriteDataUnits()) {
                _stopFlag = true;
            }

        });
    }

    void FileToFileTask::clear() noexcept
    {
        m_sourceFilePtr.reset();
        m_resultFilePtr.reset();
    }

    RetVal<TASK_CODES, std::shared_ptr<twPro::FileReaderByParts>> FileToFileTask::createFileReader(const std::string & _filePath) const noexcept
    {   
        RetVal<TASK_CODES, std::shared_ptr<twPro::FileReaderByParts>> retVal;
        retVal.result = true;

        try {
            retVal.value.reset(new twPro::FileReaderByParts(_filePath));
        }
        catch (std::exception e) {
            std::string text(e.what());
            retVal.result = std::make_tuple( TASK_CODES::ERROR, text.empty() ? "File reader returns unexpected error" : text );
        }

        return retVal;
    }

    RetVal<TASK_CODES, std::shared_ptr<twPro::FileWriterByParts>> FileToFileTask::createFileWriter(const std::string & _filePath) const noexcept
    {
        RetVal<TASK_CODES, std::shared_ptr<twPro::FileWriterByParts>> retVal;
        retVal.result = true;

        try {
            retVal.value.reset(new twPro::FileWriterByParts(_filePath));
        }
        catch (std::exception e) {
            std::string text(e.what());
            retVal.result = std::make_tuple( TASK_CODES::ERROR, text.empty() ? "File writer returns unexpected error" : text );
        }

        return retVal;
    }

    size_t FileToFileTask::totalData() const noexcept
    {
        return  m_sourceFilePtr->totalData();
    }

    size_t FileToFileTask::totalWriteDataUnits() const noexcept
    {
        return (totalData() / parameters().blockSize) + (totalData() % parameters().blockSize > 0 ? 1 : 0);
    }

}
